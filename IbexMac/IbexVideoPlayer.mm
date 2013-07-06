
//
//  IbexVideoPlayer.m
//  IbexMac
//
//  Created by Hesham Wahba on 4/27/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import "IbexVideoPlayer.h"

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>

#import "VideoPlayer.h"

@implementation IbexVideoPlayer

static Ibex::VideoPlayer *player = 0;
- (id)init
{
    self = [super init];
    if (self) {
        _videoTexture = new GLuint[2];
        player = new Ibex::VideoPlayer();
    }
    return self;
}

- (int)loadVideo:(NSString*)fileName andIsStereo:(bool)isStereo {
    if([fileName isKindOfClass:NSArray.class]) {
        NSArray *a = (NSArray*)fileName;
        fileName = a[0];
        isStereo = ((NSNumber*)a[1]).integerValue;
    }
    
    NSOpenGLContext* newContext = nil;
    newContext = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat shareContext:_share];
    [newContext makeCurrentContext];
    
    _videoTexture = player->videoTexture;
    player->playVideo(fileName.UTF8String, isStereo);
    [NSOpenGLContext clearCurrentContext];
    
    //delete []player;
    //player = 0;
    
    return 0;
}

- (int)loadCamera:(NSNumber*)cameraID andIsStereo:(bool)isStereo {
    if([cameraID isKindOfClass:NSArray.class]) {
        NSArray *a = (NSArray*)cameraID;
        cameraID = ((NSNumber*)a[0]);
        isStereo = ((NSNumber*)a[1]).integerValue;
    }
    
    NSOpenGLContext* newContext = nil;
    newContext = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat shareContext:_share];
    [newContext makeCurrentContext];
    
    _videoTexture = player->videoTexture;
    player->openCamera(isStereo, cameraID.intValue);
    [NSOpenGLContext clearCurrentContext];
    
    return 0;
}

- (GLfloat)width {
    return player->width;
}
- (GLfloat)height {
    return player->height;
}

@end
