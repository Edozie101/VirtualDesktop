
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

- (id)init
{
    self = [super init];
    if (self) {
        _videoTexture = new GLuint[2];
        _videoTexture[0] = _videoTexture[1] = 0;
        _player = 0;
    }
    return self;
}

- (int)loadVideo:(NSString*)fileName andIsStereo:(bool)isStereo {
    if([fileName isKindOfClass:NSArray.class]) {
        NSArray *a = (NSArray*)fileName;
        fileName = a[0];
        isStereo = ((NSNumber*)a[1]).integerValue;
    }
    
    if(_player) {
        delete _player;
        _player = 0;
    }
    newContext = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat shareContext:_share];
    [newContext makeCurrentContext];
    
    _player = new Ibex::VLCVideoPlayer();
    
    _videoTexture = _player->videoTexture;
    _player->playVideo(fileName.UTF8String, isStereo, 0, 0, (__bridge_retained void*)newContext);//(__bridge_retained void *)self);
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
    
    if(_player) {
        delete _player;
        _player = 0;
    }
    newContext = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat shareContext:_share];
    [newContext makeCurrentContext];
    _player = new Ibex::VLCVideoPlayer();
    
    _videoTexture = _player->videoTexture;
    _player->openCamera(isStereo, cameraID.intValue);
    [NSOpenGLContext clearCurrentContext];
    
    return 0;
}

- (GLfloat)width {
    return (_player) ? _player->width : 0;
}
- (GLfloat)height {
    return (_player) ? _player->height : 0;
}

void setupVideoGLContext(void *data) {
    NSOpenGLContext *context = (__bridge_transfer NSOpenGLContext*)data;
    [context makeCurrentContext];
}

@end
