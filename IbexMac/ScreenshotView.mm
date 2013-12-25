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

static inline void copyImageToBytes(const CGImageRef &img, const CGImageRef &cursorImageRef, GLubyte** spriteData, const bool &clear, const GLuint &imgW, const GLuint &imgH) {
    if(*spriteData == NULL) {
        return;
    }
    
    const GLuint mouseW = (GLuint)CGImageGetWidth(cursorImageRef);
	const GLuint mouseH = (GLuint)CGImageGetHeight(cursorImageRef);
    
	// Uses the bitmatp creation function provided by the Core Graphics framework.
	CGContextRef spriteContext = CGBitmapContextCreate(*spriteData, imgW, imgH, 8, imgW * 4, CGImageGetColorSpace(img), kCGImageAlphaNoneSkipLast);
	
	// Translate and scale the context to draw the image upside-down (conflict in flipped-ness between GL textures and CG contexts)
	CGContextTranslateCTM(spriteContext, 0, imgH);
	CGContextScaleCTM(spriteContext, 1, -1);
	
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
}

//- (void)createGLTextureNoAlphaUsingPBO:(GLuint *)texName, const CGImageRef &img, GLubyte** spriteData, const bool &clear)
- (void)createGLTextureNoAlphaUsingPBO:(GLuint *)texName fromCGImage:(CGImageRef)img andCursor:(CGImageRef)cursorImageRef andDataCache:(GLubyte**)spriteData andClear:(bool)clear
{
    static bool changed = true;
    //    static GLuint lastTexture = 0;
    //    bool newTexture = (*texName == 0);
    const size_t imgW = CGImageGetWidth(img);
	const size_t imgH = CGImageGetHeight(img);
    static size_t lastW = 0;
    static size_t lastH = 0;
    changed = (lastW != imgW) || (lastH != imgH);// || newTexture;
    lastW = imgW;
    lastH = imgH;
    
    // "index" is used to copy pixels from a PBO to a texture object
    // "nextIndex" is used to update pixels in the other PBO
    static int index = 0;
    static int nextIndex = 1;
    static GLuint pboIds[2] = {0, 0};
    
    //    if(newTexture) {
    //        // Use OpenGL ES to generate a name for the texture.
    //        glGenTextures(1, texName);
    //    }
    
    if(pboIds[0] == 0) {
        glGenBuffers(2, pboIds);
    }
    
    index = (index + 1) % 2;
    nextIndex = (index + 1) % 2;
    
    // bind the texture and PBO
    glBindTexture(GL_TEXTURE_2D, *texName);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[index]);
    
//    changed = true;
    if(changed) {
        changed = false;
//        glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)imgW);
//        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // copy pixels from PBO to texture object
        // Use offset instead of pointer.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, (int)imgW, (int)imgH, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    } else {
        // copy pixels from PBO to texture object
        // Use offset instead of pointer.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (int)imgW, (int)imgH, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }
    
    // bind PBO to update texture source
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboIds[nextIndex]);
    
    const size_t DATA_SIZE = imgW*imgH*4;
    
    // Note that glMapBufferARB() causes sync issue.
    // If GPU is working with this buffer, glMapBufferARB() will wait(stall)
    // until GPU to finish its job. To avoid waiting (idle), you can call
    // first glBufferDataARB() with NULL pointer before glMapBufferARB().
    // If you do that, the previous data in PBO will be discarded and
    // glMapBufferARB() returns a new allocated pointer immediately
    // even if GPU is still working with the previous data.
    glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE, 0, GL_STREAM_DRAW);
    
    // map the buffer object into client's memory
    GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if(ptr)
    {
        // update data directly on the mapped buffer
        copyImageToBytes(img, cursorImageRef, &ptr, clear, (int)imgW, (int)imgH);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release the mapped buffer
    }
    
    // it is good idea to release PBOs with ID 0 after use.
    // Once bound with 0, all pixel operations are back to normal ways.
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

- (void)createGLTextureNoAlpha:(GLuint *)texName fromCGImage:(CGImageRef)img andCursor:(CGImageRef)cursorImageRef andDataCache:(GLubyte**)spriteData andClear:(bool)clear
{
    bool newTexture = (*texName == 0);
	static CGContextRef spriteContext = nil;
	GLuint imgW, imgH, texW, texH;

    const GLuint mouseW = (GLuint)CGImageGetWidth(cursorImageRef);
	const GLuint mouseH = (GLuint)CGImageGetHeight(cursorImageRef);
    
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
    
    static CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    
	// Uses the bitmatp creation function provided by the Core Graphics framework.
    if(spriteContext == nil) {
        spriteContext = CGBitmapContextCreate(*spriteData, texW, texH, 8, texW * 4, /*CGImageGetColorSpace(img)*/space, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);
        
        // Translate and scale the context to draw the image upside-down (conflict in flipped-ness between GL textures and CG contexts)
        CGContextTranslateCTM(spriteContext, 0., texH);
        CGContextScaleCTM(spriteContext, 1., -1.);
    }
    
    // After you create the context, you can draw the sprite image to the context.
    const CGRect r = CGRectMake(0.0, 0.0, imgW, imgH);
    if(clear) {
        CGContextClearRect(spriteContext, r);
    }

	CGContextDrawImage(spriteContext, r, img);
    const CGRect mouseRect = CGRectMake(cursorPosX, cursorPosY-mouseH, mouseW, mouseH);
    CGContextDrawImage(spriteContext, mouseRect, cursorImageRef);
	// You don't need the context at this point, so you need to release it to avoid memory leaks.
//	CGContextRelease(spriteContext);
    
    //    glEnable(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)texW);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if(newTexture) {
        // Use OpenGL ES to generate a name for the texture.
        glGenTextures(1, texName);
//        glEnable(GL_TEXTURE_RECTANGLE);
        // Bind the texture name.
        glBindTexture(GL_TEXTURE_2D, *texName);
        
        // Set the texture parameters to use a minifying filter and a linear filer (weighted average)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
//        glTextureRangeAPPLE(GL_TEXTURE_2D, texH*texW*4, *spriteData);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_SHARED_APPLE);
//        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        
        // Specify a 2D texture image, providing the a pointer to the image data in memory
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, *spriteData);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, *spriteData);
    } else {
        glBindTexture(GL_TEXTURE_2D, *texName);

        // Set the texture parameters to use a minifying filter and a linear filer (weighted average)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
//        glTextureRangeAPPLE(GL_TEXTURE_2D, texH*texW*4, *spriteData);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_SHARED_APPLE);//GL_STORAGE_CACHED_APPLE);
//        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        
//        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texW, texH, GL_RGBA, GL_UNSIGNED_BYTE, *spriteData);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texW, texH, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, *spriteData);
        // to resize texture, need to check if it changed and only update then then
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, *spriteData);
    }
//    glGenerateMipmap(GL_TEXTURE_2D);
    
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
    
//    //static
//    NSCursor *systemCursor;
////    static dispatch_once_t onceToken;
////    dispatch_once(&onceToken, ^{
//        systemCursor = NSCursor.currentSystemCursor;
////    });
    [newContext makeCurrentContext];
    
    CGRect mainDisplayRect = NSScreen.mainScreen.frame;
    mainDisplayRect = CGRectMake(0, 0, physicalWidth, physicalHeight);
  
    [newContext makeCurrentContext];
    while(1) {
        [cocoaCondition lock];
        [cocoaCondition wait];
        [cocoaCondition unlock];
        
//        NSLog(@"NSScreen.mainScreen: %@, size: %@", NSScreen.mainScreen, NSStringFromRect(NSScreen.mainScreen.frame));
        done = 0;
        CFArrayRef a = CGWindowListCreate(
                                          kCGWindowListOptionOnScreenBelowWindow,
                                          (CGWindowID)_window.windowNumber
                                          );
        
        CGImageRef img = CGWindowListCreateImageFromArray(
                                                          mainDisplayRect,
                                                          a,
                                                          kCGWindowImageDefault
                                                          );
//        CGImageRef img = CGDisplayCreateImage(
//                                        kCGDirectMainDisplay//CGDirectDisplayID displayID
//                                        );
        
        CGImageRef cursorImageRef = nil;
        if(NSCursor.currentSystemCursor != nil) {
            cursorImageRef = [NSCursor.currentSystemCursor.image CGImageForProposedRect:nil context:nil hints:nil];
        }
        
        
//        [self savePNGImage:img path:@"/Users/hesh/blah_rift.png"];
//        exit(0);
        
        
        [self createGLTextureNoAlpha:&desktopTexture fromCGImage:img andCursor:cursorImageRef andDataCache:&s andClear:NO];
//        [self createGLTextureNoAlphaUsingPBO:&desktopTexture fromCGImage:img andCursor:cursorImageRef andDataCache:&s andClear:NO];
        
        glFlush();
//        [newContext flushBuffer];
        CFRelease(a);
        CGImageRelease(img);
    }
}

@end
