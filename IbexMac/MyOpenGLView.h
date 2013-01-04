//
//  MyOpenGLView.h
//  IbexMac
//
//  Created by Hesham Wahba on 12/27/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern char mResourcePath[1024];

@interface MyOpenGLView : NSOpenGLView {
    CVDisplayLinkRef displayLink; //display link for managing rendering thread
}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime;

@end
