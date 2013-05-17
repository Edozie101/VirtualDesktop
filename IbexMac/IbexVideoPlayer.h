//
//  IbexVideoPlayer.h
//  IbexMac
//
//  Created by Hesham Wahba on 4/27/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface IbexVideoPlayer : NSObject {
}

@property (retain,nonatomic) NSOpenGLPixelFormat *pixelFormat;
@property (retain,nonatomic) NSOpenGLContext *share;
@property (nonatomic)        GLuint *videoTexture;

- (int)loadVideo:(NSString*)fileName_ andIsStereo:(bool)isStereo;
- (GLfloat)width;
- (GLfloat)height;

@end
