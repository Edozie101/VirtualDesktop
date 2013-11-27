//
//  ScreenshotView.m
//  IbexMac
//
//  Created by Hesham Wahba on 4/25/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import "ScreenshotView.h"

#include "ibex.h"

@implementation ScreenshotView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
    // Drawing code here.
}

- (void)createGLTextureNoAlpha:(GLuint *)texName fromCGImage:(CGImageRef)img andCursor:(CGImageRef)cursorImageRef andDataCache:(GLubyte**)spriteData andClear:(bool)clear
{
    bool newTexture = (*texName == 0);
	CGContextRef spriteContext;
	GLuint imgW, imgH, texW, texH;

    GLuint mouseW = (GLuint)CGImageGetWidth(cursorImageRef);
	GLuint mouseH = (GLuint)CGImageGetHeight(cursorImageRef);
    
	imgW = (GLuint)CGImageGetWidth(img);
	imgH = (GLuint)CGImageGetHeight(img);
    texW = imgW;
    texH = imgH;
	
    if(*spriteData == NULL) {
        // Allocated memory needed for the bitmap context
        *spriteData = (GLubyte*) calloc(texH, texW * 4);
        NSLog(@"Allocating more memory - display: %dx%d", texW, texH);
    }
//    NSLog(@"display: %dx%d", texW, texH);
    
	// Uses the bitmatp creation function provided by the Core Graphics framework.
	spriteContext = CGBitmapContextCreate(*spriteData, texW, texH, 8, texW * 4, CGImageGetColorSpace(img), kCGImageAlphaNoneSkipLast);
	
	// Translate and scale the context to draw the image upside-down (conflict in flipped-ness between GL textures and CG contexts)
	CGContextTranslateCTM(spriteContext, 0., texH);
	CGContextScaleCTM(spriteContext, 1., -1.);
	
	// After you create the context, you can draw the sprite image to the context.
    const CGRect r = CGRectMake(0.0, 0.0, imgW, imgH);
    if(clear) {
        CGContextClearRect(spriteContext, r);
    }
	CGContextDrawImage(spriteContext, r, img);
    const CGRect mouseRect = CGRectMake(cursorPosX, cursorPosY-mouseH, mouseW, mouseH);
    CGContextDrawImage(spriteContext, mouseRect, cursorImageRef);
	// You don't need the context at this point, so you need to release it to avoid memory leaks.
	CGContextRelease(spriteContext);
	
    //    glEnable(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)texW);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if(newTexture) {
        // Use OpenGL ES to generate a name for the texture.
        glGenTextures(1, texName);
        // Bind the texture name.
        glBindTexture(GL_TEXTURE_2D, *texName);
        // Specify a 2D texture image, providing the a pointer to the image data in memory
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, *spriteData);
    } else {
        glBindTexture(GL_TEXTURE_2D, *texName);
//        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texW, texH, GL_RGBA, GL_UNSIGNED_BYTE, *spriteData);
        // to resize texture, need to check if it changed and only update then then
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, *spriteData);
    }
	// Set the texture parameters to use a minifying filter and a linear filer (weighted average)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // user-allocated, don't touch it
    //	free(*spriteData);
    //    *spriteData = 0;
}

- (void)savePNGImage:(CGImageRef)imageRef path:(NSString *)path
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

- (void)loopScreenshot {
    static NSOpenGLContext* newContext = nil;
    newContext = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat shareContext:_share];
    static GLubyte *s = NULL;
    
    //static
    NSCursor *systemCursor;
//    static dispatch_once_t onceToken;
//    dispatch_once(&onceToken, ^{
        systemCursor = NSCursor.currentSystemCursor;
//    });
    [newContext makeCurrentContext];
    
    CGRect mainDisplayRect = NSScreen.mainScreen.frame;
    mainDisplayRect = CGRectMake(0, 0, physicalWidth, physicalHeight);//1440,900);//1440, 900);
    
    while(1) {
//        NSLog(@"NSScreen.mainScreen: %@, size: %@", NSScreen.mainScreen, NSStringFromRect(NSScreen.mainScreen.frame));
        done = 0;
        [newContext makeCurrentContext];
        CGImageRef cursorImageRef = nil;
        if(NSCursor.currentSystemCursor != nil) {
            cursorImageRef = [NSCursor.currentSystemCursor.image CGImageForProposedRect:nil context:nil hints:nil];
        }
        
        CFArrayRef a = CGWindowListCreate(
                                          kCGWindowListOptionOnScreenBelowWindow,
                                          (CGWindowID)_window.windowNumber
                                          );
        
        CGImageRef img = CGWindowListCreateImageFromArray(
                                                          mainDisplayRect,
                                                          a,
                                                          kCGWindowImageDefault
                                                          );
        
//        [self savePNGImage:img path:@"/Users/hesh/blah_rift.png"];
//        exit(0);
        
        [self createGLTextureNoAlpha:&desktopTexture fromCGImage:img andCursor:cursorImageRef andDataCache:&s andClear:NO];
        
        glFlush();
//        [newContext flushBuffer];
        CFRelease(a);
        CGImageRelease(img);
        
        [cocoaCondition lock];
        [cocoaCondition wait];
        [cocoaCondition unlock];
    }
}

@end
