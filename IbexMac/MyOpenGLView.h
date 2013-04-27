//
//  MyOpenGLView.h
//  IbexMac
//
//  Created by Hesham Wahba on 12/27/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern char mResourcePath[1024];
extern NSCondition *cocoaCondition;

@class NSOpenGLContext;
@class ScreenshotView;

@interface MyOpenGLView : NSOpenGLView {
    CVDisplayLinkRef displayLink; //display link for managing rendering thread
}

@property (retain,nonatomic) NSOpenGLContext* myContext;
@property (retain,nonatomic) ScreenshotView *screenshotView;

- (GLuint)getScreenshot;
- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime;

@end
