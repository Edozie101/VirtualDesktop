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
#import "filesystem/Filesystem.h"

#include "string.h"
#include <vector>
#include <string>

static void savePNGImage(CGImageRef imageRef, NSString *path)
{
    NSURL *fileURL = [NSURL fileURLWithPath:path];
    CGImageDestinationRef dr = CGImageDestinationCreateWithURL((__bridge CFURLRef)fileURL, kUTTypePNG , 1, NULL);
    
    if(dr != nil) {
        CGImageDestinationAddImage(dr, imageRef, NULL);
        CGImageDestinationFinalize(dr);
        
        CFRelease(dr);
    } else {
        NSLog(@"File path: %@", fileURL);
        NSLog(@"ERROR saving");
        return;
    }
    //    exit(0);
}

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
extern "C" GLuint loadTexture(const char *path_, bool flip, bool isAbsolutePath, bool disableAlpha, void *myDataIn, size_t widthIn, size_t heightIn) {
    GLuint myTextureName = 0;
    
    size_t width = widthIn;
    size_t height = heightIn;
    void *myData = (myDataIn == 0) ? getImageData(path_, width, height, flip, isAbsolutePath, disableAlpha) : myDataIn;
    
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
    if(myDataIn == 0) {
        free(myData);
    }
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

GLuint createApplicationListImage(const char *path_, size_t &width, size_t &height, int &selectedX, int &selectedY) {
    const bool flip = true;
    const int iconRes = 96;
    const int iconSpacing = 16;
    std::vector<std::string> appDirectory = Filesystem::listDirectory(path_);
    
    int appCount = 0;
    for(int i = 0; i < appDirectory.size(); ++i) {
        if(appDirectory[i].find(".app") == std::string::npos) continue;
        ++appCount;
    }
    
    const int vert = 8;
    const int horiz = ceil(float(appCount)/float(vert));
    selectedX %= horiz;
    selectedY %= vert;
    
    width = horiz*(iconRes+2*iconSpacing);
    height = vert*(iconRes+2*iconSpacing);
    void * myData = calloc(width * 4, height);
    CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    CGContextRef myBitmapContext = CGBitmapContextCreate(myData,
                                                         width, height, 8,
                                                         width*4, space,
                                                         kCGBitmapByteOrder32Host |
                                                         kCGImageAlphaPremultipliedFirst);
    CGContextSetFillColorWithColor(myBitmapContext, [NSColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:1.0].CGColor);//NSColor.clearColor.CGColor);
//    CGContextClearRect(myBitmapContext, CGRectMake(0, 0, width, height));
    CGContextFillRect(myBitmapContext, CGRectMake(0, 0, width, height));
    
    for(int i = 0, count = 0; i < appDirectory.size(); ++i) {
        if(appDirectory[i].find(".app") == std::string::npos) continue;
        
        const std::string s(Filesystem::getFullPath(std::string("/Applications"), appDirectory[i].c_str()));
        const char *path = s.c_str();
        
//        std::cerr << "***** ICON FOR: " << path << std::endl;
        
        NSImage *iconImage = [[NSWorkspace sharedWorkspace] iconForFile:[NSString stringWithCString:path encoding:[NSString defaultCStringEncoding]]];
        
        // NSLog(@"%s", path);
        NSRect r = NSRectFromCGRect(CGRectMake(0.0f,0.0f,iconRes,iconRes));
        CGImageRef myImageRef = [iconImage CGImageForProposedRect:&r context:nil hints:nil];
//        savePNGImage(myImageRef, @"/Users/hesh/iconTest.png");
//        exit(0);
        
        CGRect rect = CGRectMake((count/vert)*(iconRes+2*iconSpacing)+iconSpacing, (vert-(count%vert)-1)*(iconRes+2*iconSpacing)+iconSpacing, iconRes, iconRes);
        
        if(!flip) {
            CGContextTranslateCTM(myBitmapContext, 0, iconRes+2*iconSpacing);
            CGContextScaleCTM(myBitmapContext, 1.0, -1.0);
        }
        CGContextSetBlendMode(myBitmapContext, kCGBlendModeNormal);
        CGContextDrawImage(myBitmapContext, rect, myImageRef);
        
        ++count;
    }
    // draw frame
    char path[2048];
    strcpy(path, mResourcePath);
#ifdef WIN32
    strcat(path, "\\resources\\app-launcher-selection-frame.png");
#else
    strcat(path, "/resources/app-launcher-selection-frame.png");
#endif
    NSImage *iconImage = [[NSImage alloc] initWithContentsOfFile:@(path)];
    
    // NSLog(@"%s", path);
    NSRect r = NSRectFromCGRect(CGRectMake(0.0f,0.0f,iconRes,iconRes));
    CGImageRef myImageRef = [iconImage CGImageForProposedRect:&r context:nil hints:nil];
    //        savePNGImage(myImageRef, @"/Users/hesh/iconTest.png");
    //        exit(0);
    CGRect rectFrame = CGRectMake(selectedX*(iconRes+2*iconSpacing), (vert-selectedY-1)*(iconRes+2*iconSpacing), iconRes+2*iconSpacing, iconRes+2*iconSpacing);
    
    if(!flip) {
        CGContextTranslateCTM(myBitmapContext, 0, iconRes+2*iconSpacing);
        CGContextScaleCTM(myBitmapContext, 1.0, -1.0);
    }
    CGContextSetBlendMode(myBitmapContext, kCGBlendModeNormal);
    CGContextDrawImage(myBitmapContext, rectFrame, myImageRef);
    ////////////////////////
    
    
    CGContextRelease(myBitmapContext);
    CGColorSpaceRelease(space);
    
    GLuint texture = loadTexture("", false, false, false, myData, width, height);
//    GLuint texture = loadTexture("/resources/humus-skybox/smaller/negz.jpg");
    free(myData);
    
    return texture;
}

//void loadApplicationIcons() {
//    std::vector<std::string> listOfApplicationPaths;
//    for(int i = 0; i < listOfApplicationPaths.size(); ++i) {
//        NSString *path = [NSString stringWithCString:listOfApplicationPaths[i].c_str() encoding:[NSString defaultCStringEncoding]];
//        NSImage *iconImage = [[NSWorkspace sharedWorkspace] iconForFile:path];
//    }
//}


@implementation IbexMacUtils

@end
