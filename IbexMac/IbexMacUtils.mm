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
#import <OpenGL/gl.h>
#import <Foundation/Foundation.h>

#import "ibex_mac_utils.h"

#include "string.h"

extern "C" GLuint loadTexture(const char *path_) {
    GLuint myTextureName;
    char path[2048];
    strcpy(path, mResourcePath);
    strcat(path, path_);
    
    
//    NSLog(@"%s", path);
    NSURL *URL = [NSURL fileURLWithPath:[NSString stringWithCString:path encoding:NSASCIIStringEncoding]];
    NSLog(@"%@", URL);
    CFURLRef url = (__bridge CFURLRef)URL;
    CGImageSourceRef myImageSourceRef = CGImageSourceCreateWithURL(url, NULL);
    CGImageRef myImageRef = CGImageSourceCreateImageAtIndex (myImageSourceRef, 0, NULL);

    size_t width = CGImageGetWidth(myImageRef);
    size_t height = CGImageGetHeight(myImageRef);
    CGRect rect = CGRectMake(0, 0, width, height);
    void * myData = calloc(width * 4, height);
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef myBitmapContext = CGBitmapContextCreate (myData,
                                                          width, height, 8,
                                                          width*4, space,
                                                          kCGBitmapByteOrder32Host |
                                                          kCGImageAlphaPremultipliedFirst);
    CGContextTranslateCTM(myBitmapContext, 0, height);
    CGContextScaleCTM(myBitmapContext, 1.0, -1.0);
    CGContextSetBlendMode(myBitmapContext, kCGBlendModeCopy);
    CGContextDrawImage(myBitmapContext, rect, myImageRef);
    CGContextRelease(myBitmapContext);
    
    glPushClientAttrib(GL_UNPACK_ROW_LENGTH);
    glPushClientAttrib(GL_UNPACK_ALIGNMENT);
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)width);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &myTextureName);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, myTextureName);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLint)width, (GLint)height,
                 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, myData);
    free(myData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glPopClientAttrib();
    glPopClientAttrib();
    
    return myTextureName;
}

@implementation IbexMacUtils

@end
