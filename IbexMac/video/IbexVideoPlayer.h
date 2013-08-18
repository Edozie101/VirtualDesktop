//
//  IbexVideoPlayer.h
//  IbexMac
//
//  Created by Hesham Wahba on 4/27/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <OpenGL/OpenGL.h>
#import <AppKit/AppKit.h>

@interface IbexVideoPlayer : NSObject {
    NSOpenGLContext* newContext;
}

@property (retain,nonatomic) NSOpenGLPixelFormat *pixelFormat;
@property (retain,nonatomic) NSOpenGLContext *share;
@property (nonatomic)        GLuint *videoTexture;

- (int)loadVideo:(NSString*)fileName_ andIsStereo:(bool)isStereo;
- (int)loadCamera:(NSNumber*)cameraID andIsStereo:(bool)isStereo;
- (GLfloat)width;
- (GLfloat)height;

void setupVideoGLContext(void *data);

@end
