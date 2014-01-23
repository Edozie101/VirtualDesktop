//
//  ScreenshotView.m
//  IbexMac
//
//  Created by Hesham Wahba on 4/25/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import "ScreenshotView.h"

#import "monitor/IbexMonitor.h"
#import "simpleworld_plugin/SimpleWorldRendererPlugin.h"

#include "ibex.h"

@implementation ScreenshotView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        spriteContext = nil;
        newContext = nil;
        spriteData = NULL;
    }
    
    return self;
}

- (void)dealloc
{
    if(spriteContext != nil) {
        CGContextRelease(spriteContext);
    }
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

- (void)createGLTextureNoAlpha:(GLuint *)texName fromCGImage:(CGImageRef)img andCursor:(CGImageRef)cursorImageRef andDataCache:(GLubyte**)spriteData_ andClear:(bool)clear andSpriteContext:(CGContextRef*)spriteContext_ andScreenRect:(CGRect)rect andHotspot:(NSPoint)hotspot andCursorPosition:(CGPoint)cursor andScaleFactor:(float)scaleFactor
{
    bool newTexture = (*texName == 0);
	GLuint imgW, imgH, texW, texH;

    const GLuint mouseW = (GLuint)CGImageGetWidth(cursorImageRef);
	const GLuint mouseH = (GLuint)CGImageGetHeight(cursorImageRef);
    
	imgW = (GLuint)CGImageGetWidth(img);
	imgH = (GLuint)CGImageGetHeight(img);
    texW = imgW;
    texH = imgH;
	
    if(*spriteData_ == 0) {
        // Allocated memory needed for the bitmap context
        *spriteData_ = (GLubyte*) calloc(texH, texW * 4);
        NSLog(@"Allocating more memory - display: %dx%d", texW, texH);
    }
//    NSLog(@"display: %dx%d", texW, texH);
    
    static CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
    
	// Uses the bitmatp creation function provided by the Core Graphics framework.
    if(*spriteContext_ == nil) {
        *spriteContext_ = CGBitmapContextCreate(*spriteData_, texW, texH, 8, texW * 4, space, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);
        
        // Translate and scale the context to draw the image upside-down (conflict in flipped-ness between GL textures and CG contexts)
        CGContextTranslateCTM(*spriteContext_, 0., texH);
        CGContextScaleCTM(*spriteContext_, 1., -1.);
    }
    
    // After you create the context, you can draw the sprite image to the context.
    const CGRect r = CGRectMake(0.0, 0.0, imgW, imgH);
    if(clear) {
        CGContextClearRect(*spriteContext_, r);
    }

	CGContextDrawImage(*spriteContext_, r, img);
    
    if(CGRectContainsPoint(rect, cursor)) {
        const CGRect mouseRect = CGRectMake((cursor.x-hotspot.x-rect.origin.x)*scaleFactor, (rect.size.height-cursor.y+hotspot.y+rect.origin.y-mouseH)*scaleFactor, mouseW*scaleFactor, mouseH*scaleFactor);
        CGContextDrawImage(*spriteContext_, mouseRect, cursorImageRef);
    }
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)texW);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if(newTexture) {
        // Use OpenGL ES to generate a name for the texture.
        glGenTextures(1, texName);
        // Bind the texture name.
        glBindTexture(GL_TEXTURE_2D, *texName);
        
        // Set the texture parameters to use a minifying filter and a linear filer (weighted average)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, *spriteData_);
    } else {
        glBindTexture(GL_TEXTURE_2D, *texName);

        // Set the texture parameters to use a minifying filter and a linear filer (weighted average)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texW, texH, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, *spriteData_);
    }
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
    newContext = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat shareContext:_share];
    [newContext makeCurrentContext];
    
    CGRect mainDisplayRect = NSScreen.mainScreen.frame;
    //mainDisplayRect = CGRectMake(0, 0, physicalWidth, physicalHeight);
    NSScreen *mainScreen = NSScreen.mainScreen;
  
    [newContext makeCurrentContext];
    while(1) {
        [cocoaCondition lock];
        [cocoaCondition wait];
        [cocoaCondition unlock];
        
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
        
        CGEventRef event = CGEventCreate(NULL);
        CGPoint cursor = CGEventGetLocation(event);
        CFRelease(event);
        CGImageRef cursorImageRef = nil;
        if(NSCursor.currentSystemCursor != nil) {
            cursorImageRef = [NSCursor.currentSystemCursor.image CGImageForProposedRect:nil context:nil hints:nil];
        }
        
        GLuint desktopTexture = 0;
        [self createGLTextureNoAlpha:&desktopTexture fromCGImage:img andCursor:cursorImageRef andDataCache:&spriteData andClear:NO andSpriteContext:&spriteContext andScreenRect:mainDisplayRect andHotspot:CGPointZero andCursorPosition:cursor andScaleFactor:mainScreen.backingScaleFactor];
        
        glFlush();
        CFRelease(a);
        CGImageRelease(img);
    }
}

- (void)initMonitorScreens {
    screens = [NSArray arrayWithArray:NSScreen.screens];
    CGDirectDisplayID *onlineDisplays = new CGDirectDisplayID[32];
    uint32_t displayCount;
    CGGetOnlineDisplayList(32, onlineDisplays, &displayCount);
    for(int i = 0; i < displayCount; ++i) {
        const CGRect r = CGDisplayBounds(onlineDisplays[i]);
        ibexMonitor->desktopRects.push_back(RECT(r.origin.x,r.origin.y,r.origin.x+r.size.width,r.origin.y+r.size.height));
        bool found = false;
        for(NSScreen *screen in NSScreen.screens) {
            CGDirectDisplayID screenId = ((NSNumber*)[screen.deviceDescription objectForKey:@"NSScreenNumber"]).unsignedIntValue;
            if(screenId == onlineDisplays[i]) {
                found = true;
                ibexMonitor->desktopScaleFactors.push_back(screen.backingScaleFactor);
            }
        }
        if(!found) {
            ibexMonitor->desktopScaleFactors.push_back(1.0f);
        }
        cgRects.push_back(r);
        ibexMonitor->bitmapCache.push_back(0);
    }
    
    ibexMonitor->initializeTextures();
}

- (void)loopScreenshotAllDesktops {
    newContext = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat shareContext:_share];
    [newContext makeCurrentContext];
    
    [self initMonitorScreens];
    
    std::vector<CGContextRef> spriteContexts;
    for(int i = 0; i < screens.count; ++i) {
        spriteContexts.push_back(0);
    }
    while(1) {
        [cocoaCondition lock];
        [cocoaCondition wait];
        [cocoaCondition unlock];
        
        done = 0;
        const CFArrayRef a = CGWindowListCreate(
                                          kCGWindowListOptionOnScreenBelowWindow,
                                          (CGWindowID)_window.windowNumber
                                          );
        
        const CGEventRef event = CGEventCreate(NULL);
        const CGPoint cursor = CGEventGetLocation(event);
        CFRelease(event);
        CGImageRef cursorImageRef = nil;
        NSPoint hotspot = CGPointZero;
        if(NSCursor.currentSystemCursor != nil) {
            cursorImageRef = [NSCursor.currentSystemCursor.image CGImageForProposedRect:nil context:nil hints:nil];
            hotspot = NSCursor.currentSystemCursor.hotSpot;
        }
        
        
        for(int i = 0; i < screens.count; ++i) {
            const NSScreen *screen = screens[i];
            
            // screen.frame is WRONG
            const CGImageRef img = CGWindowListCreateImageFromArray(
                                                              cgRects[i],//screen.frame, //CGRectMake(1440,260,1136,640),
                                                              a,
                                                              kCGWindowImageDefault
                                                              );
            
//            if(i == 1) {
//            [self savePNGImage:img path:[NSString stringWithFormat:@"/Users/hesh/%d.png",i]];
//                exit(0);
//            }
            
            GLubyte** _spriteData = &ibexMonitor->bitmapCache[i];
            [self createGLTextureNoAlpha:&ibexMonitor->desktopTextures[i] fromCGImage:img andCursor:cursorImageRef andDataCache:_spriteData andClear:NO andSpriteContext:&spriteContexts[i] andScreenRect:cgRects[i] andHotspot:hotspot andCursorPosition:cursor andScaleFactor:screen.backingScaleFactor];
            
            CGImageRelease(img);
        }
        glFlush();
        CFRelease(a);
        
//        exit(0);
    }
}

@end
