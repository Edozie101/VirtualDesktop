//
//  IbexVideoPlayer.h
//  IbexMac
//
//  Created by Hesham Wahba on 4/27/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import "../opengl_helpers.h"

#import <Foundation/Foundation.h>
#import <OpenGL/OpenGL.h>
#import <AppKit/AppKit.h>

#import "VLCVideoPlayer.h"

@interface IbexVideoPlayer : NSObject {
    NSOpenGLContext* newContext;
}

@property (retain,nonatomic) NSOpenGLPixelFormat *pixelFormat;
@property (retain,nonatomic) NSOpenGLContext *share;
@property (nonatomic)        GLuint staticTexture;
@property (nonatomic)        GLuint *staticTextures;
@property (nonatomic)        GLuint *videoTexture;
@property (nonatomic)        GLuint isSBS;
@property (nonatomic)        Ibex::VLCVideoPlayer *player;

- (bool)shouldPlayStatic;
- (int)loadVideo:(NSString*)fileName andIsStereo:(bool)isStereo andIsSBS:(unsigned int)isSBS;
- (int)loadCamera:(NSNumber*)cameraID andIsStereo:(bool)isStereo;
- (GLfloat)width;
- (GLfloat)height;

void setupVideoGLContext(void *data);

@end
