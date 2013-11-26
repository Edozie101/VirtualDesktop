
//
//  IbexVideoPlayer.m
//  IbexMac
//
//  Created by Hesham Wahba on 4/27/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import "IbexVideoPlayer.h"

#import "VLCVideoPlayer.h"

@implementation IbexVideoPlayer

static Ibex::VLCVideoPlayer *player = 0;
- (id)init
{
    self = [super init];
    if (self) {
        _videoTexture = new GLuint[2];
        player = new Ibex::VLCVideoPlayer();
    }
    return self;
}

- (int)loadVideo:(NSString*)fileName andIsStereo:(bool)isStereo {
    if([fileName isKindOfClass:NSArray.class]) {
        NSArray *a = (NSArray*)fileName;
        fileName = a[0];
        isStereo = ((NSNumber*)a[1]).integerValue;
    }
    
    newContext = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat shareContext:_share];
    [newContext makeCurrentContext];
    
    _videoTexture = player->videoTexture;
    player->playVideo(fileName.UTF8String, isStereo, 0, 0, (__bridge_retained void*)newContext);//(__bridge_retained void *)self);
    //[NSOpenGLContext clearCurrentContext];
    
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

void setupVideoGLContext(void *data) {
    NSOpenGLContext *context = (__bridge_transfer NSOpenGLContext*)data;
    [context makeCurrentContext];
}

@end
