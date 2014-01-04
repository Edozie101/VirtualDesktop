//
//  ScreenshotView.h
//  IbexMac
//
//  Created by Hesham Wahba on 4/25/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "opengl_helpers.h"

#import <Cocoa/Cocoa.h>

extern NSCondition *cocoaCondition;

@interface ScreenshotView : NSView {
    CGContextRef spriteContext;
    NSOpenGLContext* newContext;
    GLubyte *spriteData;
    
    NSArray *screens;
}

@property (retain,nonatomic) NSOpenGLPixelFormat *pixelFormat;
@property (retain,nonatomic) NSOpenGLContext *share;

- (void)loopScreenshot;
- (void)loopScreenshotAllDesktops;

@end
