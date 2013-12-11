//
//  IbexMacUtils.m
//  IbexMac
//
//  Created by Hesham Wahba on 1/1/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//
#import "IbexMacUtils.h"

#include <objc/objc.h>
#include <objc/message.h>
#include <Foundation/Foundation.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <Foundation/Foundation.h>

#import "ibex_mac_utils.h"

#include "string.h"

static void *getImageData(const char *path_, size_t &width, size_t &height, bool flip, bool isAbsolutePath, bool disableAlpha) {
    char path[2048];
    if(isAbsolutePath) {
        strcpy(path, path_);
    } else {
        strcpy(path, mResourcePath);
        strcat(path, path_);
    }

    // NSLog(@"%s", path);
    NSURL *URL = [NSURL fileURLWithPath:[NSString stringWithCString:path encoding:NSASCIIStringEncoding]];
    NSLog(@"%@", URL);
    CFURLRef url = (__bridge CFURLRef)URL;
    CGImageSourceRef myImageSourceRef = CGImageSourceCreateWithURL(url, NULL);
    CGImageRef myImageRef = CGImageSourceCreateImageAtIndex (myImageSourceRef, 0, NULL);
    
    width = CGImageGetWidth(myImageRef);
    height = CGImageGetHeight(myImageRef);
    CGRect rect = CGRectMake(0, 0, width, height);
    void * myData = calloc(width * 4, height);
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef myBitmapContext = CGBitmapContextCreate(myData,
                                                         width, height, 8,
                                                         width*4, space,
                                                         kCGBitmapByteOrder32Host |
                                                         ((disableAlpha) ? kCGImageAlphaNoneSkipFirst : kCGImageAlphaPremultipliedFirst));
    if(!flip) {
        CGContextTranslateCTM(myBitmapContext, 0, height);
        CGContextScaleCTM(myBitmapContext, 1.0, -1.0);
    }
    CGContextSetBlendMode(myBitmapContext, kCGBlendModeCopy);
    CGContextDrawImage(myBitmapContext, rect, myImageRef);
    CGContextRelease(myBitmapContext);
    
    CGColorSpaceRelease(space);
    CGImageRelease(myImageRef);
    CFRelease(myImageSourceRef);
    
    return myData;
}

extern "C" GLuint loadNormalTexture(const char *path_) {
    return loadTexture(path_, false, false, true);
}
extern "C" GLuint loadTexture(const char *path_, bool flip, bool isAbsolutePath, bool disableAlpha) {
    GLuint myTextureName;
    
    size_t width = 0;
    size_t height = 0;
    void *myData = getImageData(path_, width, height, flip, isAbsolutePath, disableAlpha);
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)width);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &myTextureName);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, myTextureName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    //if(glIsExtSupported("GL_EXT_texture_filter_anisotropic")) {
    GLfloat fLargest;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
    //}
    
    glTexImage2D(GL_TEXTURE_2D, 0, ((disableAlpha) ? GL_RGB8 : GL_RGBA8), (GLint)width, (GLint)height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, myData);
    glGenerateMipmap(GL_TEXTURE_2D);
    free(myData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return myTextureName;
}

extern "C" GLuint loadCubemapTextures(const char *path_[6]) {
    GLuint myTextureName = 0;
    
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    glGenTextures(1, &myTextureName);
    glBindTexture(GL_TEXTURE_CUBE_MAP, myTextureName);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // GL_LINEAR
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
    
    //if(glIsExtSupported("GL_EXT_texture_filter_anisotropic")) {
        GLfloat fLargest;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
    //}
    
    for(int i = 0; i < 6; ++i) {
        size_t width = 0;
        size_t height = 0;
        void *myData = getImageData(path_[i], width, height, true, false, true);
        
        glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)width);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGB8, (GLint)width, (GLint)height,
                     0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, myData);
        free(myData);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    return myTextureName;
}

@implementation IbexMacUtils

@end
