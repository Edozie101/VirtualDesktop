//
//  MyOpenGLView.m
//  IbexMac
//
//  Created by Hesham Wahba on 12/27/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

//#include "opengl_helpers.h"

#import <QuartzCore/QuartzCore.h>

#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>

#import <CoreVideo/CVDisplayLink.h>

#import <mach/mach_time.h>
#import <libkern/OSAtomic.h>
//#import <Opengl/CGLTypes.h>
//#import <OpenGL/CGLMacro.h>

#import <OpenGL/glext.h>
#import "MyOpenGLView.h"

#import <Carbon/Carbon.h>

#include "ibex.h"

#import <OpenGL/OpenGL.h>

char mResourcePath[1024];

@implementation MyOpenGLView

static Ibex *ibex = nil;

EventHandlerUPP hotKeyFunction;

- (void)controlDesktopUpdate {
    CGDisplayHideCursor(kCGDirectMainDisplay);
    if(controlDesktop) {
        SetSystemUIMode (kUIModeNormal, kUIOptionDisableProcessSwitch);
        [_window setIgnoresMouseEvents:YES];
        [_window setAcceptsMouseMovedEvents:NO];
    } else {
        SetSystemUIMode (kUIModeContentSuppressed, kUIOptionDisableProcessSwitch);
        
        [_window setIgnoresMouseEvents:NO];
        [_window setAcceptsMouseMovedEvents:YES];
        
        [_window makeKeyAndOrderFront:_window];
        [_window makeKeyAndOrderFront:self];
        [NSApp activateIgnoringOtherApps:YES];
    }
}
OSStatus hotKeyHandler(EventHandlerCallRef nextHandler,EventRef theEvent, void *userData)
{
    MyOpenGLView *obj =  (__bridge MyOpenGLView*)userData;//(__bridge MyOpenGLView*)userData;
    controlDesktop = !controlDesktop;
    
    [obj controlDesktopUpdate];
    
    return noErr;
}


- (void) registerHotkey {
    //handler
    hotKeyFunction = NewEventHandlerUPP(hotKeyHandler);
    EventTypeSpec eventType;
    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyReleased;
    InstallApplicationEventHandler(hotKeyFunction,1,&eventType,(__bridge_retained void*)self,NULL);
    //hotkey
    UInt32 keyCode = kVK_F1;//kVK_ANSI_R; //F19
    EventHotKeyRef theRef = NULL;
    EventHotKeyID keyID;
    keyID.signature = 'FOO '; //arbitrary string
    keyID.id = 1;
    RegisterEventHotKey(keyCode,/*cmdKey+*/shiftKey,keyID,GetApplicationEventTarget(),0,&theRef);
}
- (void) unregisterHotkey {
    // release self one more time...
}

NSTimer *t;

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format {
    self = [super initWithFrame:frameRect pixelFormat:format];
    if (self) {
//        CGLError err;// = 0;
////        CGLContextObj ctx = CGLGetCurrentContext();
//        
//        NSOpenGLContext* context;
//        if(context == nil)
//            context = [self openGLContext];
//        
//        // Enable the multi-threading
//        err =  CGLEnable( /*ctx*/ *((CGLContextObj*)context.CGLContextObj), kCGLCEMPEngine);
//        
//        if (err != kCGLNoError )
//        {
//            // Multi-threaded execution is possibly not available
//            // Insert your code to take appropriate action
//            NSLog(@"Multhreaded OpenGL enabled");
//        }

        
        [self registerHotkey];
        [self controlDesktopUpdate];
        
//        [self performSelectorInBackground:@selector(loopScreenshot) withObject:nil];
    }
    
    return self;
}

// This is the renderer output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    CVReturn result = [((__bridge MyOpenGLView*)displayLinkContext) getFrameForTime:outputTime];
    return result;
}

- (void)dealloc
{
    // Release the display link
    CVDisplayLinkRelease(displayLink);
}

NSTimer *renderTimer;
// Put our timer in -awakeFromNib, so it can start up right from the beginning
- (void)prepareOpenGL
{
    [self controlDesktopUpdate];
    
    strcpy(mResourcePath, [[[NSBundle mainBundle] resourcePath]
                                 cStringUsingEncoding:NSUTF8StringEncoding]);
    
    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    // Create a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    
    // Set the renderer output callback function
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, (void*)CFBridgingRetain(self));
    
    // Set the display link for the current renderer
    CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    
    // Activate the display link
    CVDisplayLinkStart(displayLink);
}

static int desktopTextureIndex = 0;
static GLuint desktopTextures[2] = {NULL, NULL};
bool done = 0;
- (void)loopScreenshot {
//    return;
    
    static NSOpenGLContext* newContext = nil;
    newContext = [[NSOpenGLContext alloc] initWithFormat:self.pixelFormat shareContext:self.openGLContext];
    
    static GLubyte *cursorData = NULL;
    static GLubyte *s = NULL;
    
    while(1) {
            //            NSLog(@"Drawing desktop");
            [newContext makeCurrentContext];
            
            cursorPos = NSEvent.mouseLocation;
            CGPoint hotSpot = NSCursor.currentSystemCursor.hotSpot;
            cursorPos.x -= hotSpot.x;
            cursorPos.y += hotSpot.y;
            CGImageRef cursorImage = [NSCursor.currentSystemCursor.image CGImageForProposedRect:nil context:nil hints:nil];
            [self createGLTexture:&cursor fromCGImage:cursorImage andDataCache:&cursorData andClear:YES];
            
            CGImageRelease(cursorImage);
            
            
            CFArrayRef a = CGWindowListCreate(
                                              kCGWindowListOptionOnScreenBelowWindow,
                                              (CGWindowID)_window.windowNumber
                                              );
            
            CGImageRef img = CGWindowListCreateImageFromArray(
                                                              CGRectInfinite,
                                                              a,
                                                              kCGWindowImageDefault
                                                              );
            CFRelease(a);
            
            
            //            [self savePNGImage:img path:@"/Users/hesh/file.png"];
            [self createGLTexture:&desktopTexture fromCGImage:img andDataCache:&s andClear:NO];
            //            glFlush();
            
            [newContext flushBuffer];
            
            CGImageRelease(img);
            
            //            NSLog(@"drawing desktop done");
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
        NSLog(@"ERROR saving");
    }
}

- (void)loopScreenshot_multipleBuffers {
    static NSOpenGLContext* context = nil;
    if(context == nil)
        context = [self openGLContext];
    
    static NSOpenGLContext* newContext = nil;
    newContext = [[NSOpenGLContext alloc] initWithFormat:self.pixelFormat shareContext:self.openGLContext];
    
    static GLubyte *cursorData = NULL;
    static GLubyte *s = NULL;
    static GLubyte *s2 = NULL;
    static int index = 0;

    while(1) {
//        if(!done)
        {
//            NSLog(@"Drawing desktop");
            [newContext makeCurrentContext];
            
            cursorPos = NSEvent.mouseLocation;
            CGPoint hotSpot = NSCursor.currentSystemCursor.hotSpot;
            cursorPos.x -= hotSpot.x;
            cursorPos.y += hotSpot.y;
            CGImageRef cursorImage = [NSCursor.currentSystemCursor.image CGImageForProposedRect:nil context:nil hints:nil];
            [self createGLTexture:&cursor fromCGImage:cursorImage andDataCache:&cursorData andClear:YES];
            
            CGImageRelease(cursorImage);
            
            
            CFArrayRef a = CGWindowListCreate(
                                              kCGWindowListOptionOnScreenBelowWindow,
                                              (CGWindowID)_window.windowNumber
                                              );
            
            CGImageRef img = CGWindowListCreateImageFromArray(
                                                            CGRectInfinite,
                                                            a,
                                                            kCGWindowImageDefault
                                                            );
            CFRelease(a);
            

//            [self savePNGImage:img path:@"/Users/hesh/file.png"];
            
            [self createGLTexture:&desktopTextures[index] fromCGImage:img andDataCache:((index)?&s2:&s) andClear:NO];
//            glFlush();
            
            [newContext flushBuffer];            
            
            CGImageRelease(img);
            
            
            desktopTextureIndex = index;
            done = 1;
            index = (index+1)%2;
//            NSLog(@"drawing desktop done");
        }
    }
}
- (void)loopScreenshot_single {
    return;
    
    GLubyte *s = NULL;
    GLubyte **spriteData = &s;
    
    bool clear = false;
    while(1) {
            CFArrayRef a = CGWindowListCreate(
                                              kCGWindowListOptionOnScreenBelowWindow,
                                              (CGWindowID)_window.windowNumber
                                              );
            
            CGImageRef img = CGWindowListCreateImageFromArray(
                                                              CGRectInfinite,
                                                              a,
                                                              kCGWindowImageDefault
                                                              );
            CFRelease(a);
            
            //	GLubyte *spriteData = NULL;
            CGContextRef spriteContext;
            GLuint imgW, imgH, texW, texH;
            
            imgW = CGImageGetWidth(img);
            imgH = CGImageGetHeight(img);
            
            // Find smallest possible powers of 2 for our texture dimensions
            //	for (texW = 1; texW < imgW; texW *= 2) ;
            //	for (texH = 1; texH < imgH; texH *= 2) ;
            texW = imgW;
            texH = imgH;
            
            if(*spriteData == NULL) {
                // Allocated memory needed for the bitmap context
                *spriteData = (GLubyte*) calloc(texH, texW * 4);
                NSLog(@"Allocating more memory");
            }
            
            // Uses the bitmatp creation function provided by the Core Graphics framework.
            spriteContext = CGBitmapContextCreate(*spriteData, texW, texH, 8, texW * 4, CGImageGetColorSpace(img), kCGImageAlphaPremultipliedLast);
            
            // Translate and scale the context to draw the image upside-down (conflict in flipped-ness between GL textures and CG contexts)
            CGContextTranslateCTM(spriteContext, 0., texH);
            CGContextScaleCTM(spriteContext, 1., -1.);
            
            // After you create the context, you can draw the sprite image to the context.
            const CGRect r = CGRectMake(0.0, 0.0, imgW, imgH);
            if(clear) {
                CGContextClearRect(spriteContext, r);
            }
            CGContextDrawImage(spriteContext, r, img);
            // You don't need the context at this point, so you need to release it to avoid memory leaks.
            CGContextRelease(spriteContext);
            
            CGImageRelease(img);
            
            free(s);
            s = 0;
    }
}
- (void)createGLTexture:(GLuint *)texName fromCGImage:(CGImageRef)img andDataCache:(GLubyte**)spriteData andClear:(bool)clear
{
//    NSLog(@"++++++++++ %lu, %lu", *texName, (*spriteData));
    
    bool newTexture = (*texName == 0);
//	GLubyte *spriteData = NULL;
	CGContextRef spriteContext;
	GLuint imgW, imgH, texW, texH;

	imgW = CGImageGetWidth(img);
	imgH = CGImageGetHeight(img);
//    imgW = width;
//	imgH = height;
	
	// Find smallest possible powers of 2 for our texture dimensions
//	for (texW = 1; texW < imgW; texW *= 2) ;
//	for (texH = 1; texH < imgH; texH *= 2) ;
    texW = imgW;
    texH = imgH;
	
    if(*spriteData == NULL) {
        // Allocated memory needed for the bitmap context
        *spriteData = (GLubyte*) calloc(texH, texW * 4);
        NSLog(@"Allocating more memory");
    }

	// Uses the bitmatp creation function provided by the Core Graphics framework.
	spriteContext = CGBitmapContextCreate(*spriteData, texW, texH, 8, texW * 4, CGImageGetColorSpace(img), kCGImageAlphaPremultipliedLast);
	
	// Translate and scale the context to draw the image upside-down (conflict in flipped-ness between GL textures and CG contexts)
	CGContextTranslateCTM(spriteContext, 0., texH);
	CGContextScaleCTM(spriteContext, 1., -1.);
	
	// After you create the context, you can draw the sprite image to the context.
    const CGRect r = CGRectMake(0.0, 0.0, imgW, imgH);
    if(clear) {
        CGContextClearRect(spriteContext, r);
    }
	CGContextDrawImage(spriteContext, r, img);
	// You don't need the context at this point, so you need to release it to avoid memory leaks.
	CGContextRelease(spriteContext);
	
    glEnable(GL_TEXTURE_2D);
    if(newTexture) {
//        NSLog(@"+++++++++++++++++ New Texture");
        // Use OpenGL ES to generate a name for the texture.
        glGenTextures(1, texName);
        // Bind the texture name.
        glBindTexture(GL_TEXTURE_2D, *texName);
	// Specify a 2D texture image, providing the a pointer to the image data in memory
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, *spriteData);
    } else {
        glBindTexture(GL_TEXTURE_2D, *texName);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texW, texH, GL_RGBA, GL_UNSIGNED_BYTE, *spriteData);
    }
	// Set the texture parameters to use a minifying filter and a linear filer (weighted average)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	

    // user-allocated, don't touch it
//	free(*spriteData);
//    *spriteData = 0;
}

- (void) refresh {
    NSLog(@"REFRESH");
    [self setNeedsDisplay:YES];
}

//GLuint cursor(0);
CGPoint cursorPos;
- (GLuint)getScreenshot {
    {
        cursorPos = NSEvent.mouseLocation;
        CGPoint hotSpot = NSCursor.currentSystemCursor.hotSpot;
        cursorPos.x -= hotSpot.x;
        cursorPos.y += hotSpot.y;
    }
    
    return desktopTexture;
//
//    if(done) {
//        desktopTexture = desktopTextures[desktopTextureIndex];
//        done = 0;
//    }
//    return desktopTexture;
    
    static GLubyte *desktopData = NULL;
    static GLubyte *cursorData = NULL;
//    if(desktopTexture) {
//        glDeleteTextures( 1, &desktopTexture);
//        desktopTexture = 0;
//    }
//    if(cursor) {
//        glDeleteTextures( 1, &cursor );
//        cursor = 0;
//    }

    cursorPos = NSEvent.mouseLocation;
    CGPoint hotSpot = NSCursor.currentSystemCursor.hotSpot;
    cursorPos.x -= hotSpot.x;
    cursorPos.y += hotSpot.y;
    CGImageRef cursorImage = [NSCursor.currentSystemCursor.image CGImageForProposedRect:nil context:nil hints:nil];
    [self createGLTexture:&cursor fromCGImage:cursorImage andDataCache:&cursorData andClear:YES];
    
    CGImageRelease(cursorImage);
    
    CFArrayRef a = CGWindowListCreate(
                                      kCGWindowListOptionOnScreenBelowWindow,
                                      (CGWindowID)_window.windowNumber
                                      );
    
    CGImageRef i
    = CGWindowListCreateImageFromArray(
                                                    CGRectInfinite,
                                                    a,
                                                    kCGWindowImageDefault
                                                    );
    CFRelease(a);

    [self createGLTexture:&desktopTexture fromCGImage:i andDataCache:&desktopData andClear:NO];
    CGImageRelease(i);
    return desktopTexture;
}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)time
{
    static double fpsTime = 0;
    static int64_t startTime = time->videoTime;
//    NSLog(@"getFrameForTime start");
    GLfloat timeDiff = (GLfloat)(time->videoTime-startTime) / (GLfloat)(time->videoTimeScale);
    startTime = time->videoTime;
    
    static int64_t frame = 0;
    ++frame;
    fpsTime += timeDiff;
    if(fpsTime >= 5.0) {
        NSLog(@"FPS: %4.2f", ((double)frame)/fpsTime);
        frame = 0;
        fpsTime = 0;
    }
    
    //    NSLog(@"Start");
    static NSOpenGLContext* context;
    if(context == nil)
        context = [self openGLContext];
    // Drawing code here.
    
    
//    CGLLockContext(*(CGLContextObj*)context.CGLContextObj);
    
    [context makeCurrentContext];
    
//    glGenTextures(2, desktopTextures);
    glFlush();
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [self performSelectorInBackground:@selector(loopScreenshot) withObject:nil];
    });
    
    
    // Add your drawing codes here
    if(ibex == nil) {
        ibex = new Ibex(0,nil);
    }
    
    [self getScreenshot];
    cursorPosX = cursorPos.x;
    cursorPosY = cursorPos.y;
    
    ibex->render(timeDiff);
    
//    glFlush();
    
    [context flushBuffer];
//    checkForErrors();
    
//    CGLUnlockContext(*(CGLContextObj*)context.CGLContextObj);
    
    return kCVReturnSuccess;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyDown:(NSEvent *)theEvent {
    switch(theEvent.keyCode) {
        case kVK_ANSI_W:
            walkForward = 1;
            break;
        case kVK_ANSI_S:
            walkForward = -1;
            break;
        case kVK_ANSI_A:
            strafeRight = -1;
            break;
        case kVK_ANSI_D:
            strafeRight = 1;
            break;
        case kVK_Space:
            break;
    }
}
- (void)keyUp:(NSEvent *)theEvent {
    switch(theEvent.keyCode) {
        case kVK_ANSI_B:
            barrelDistort = !barrelDistort;
            break;
        case kVK_ANSI_G:
            showGround = !showGround;
            break;
        case kVK_ANSI_W:
            walkForward = 0;
            break;
        case kVK_ANSI_S:
            walkForward = 0;
            break;
        case kVK_ANSI_A:
            strafeRight = 0;
            break;
        case kVK_ANSI_D:
            strafeRight = 0;
            break;
        case kVK_Space:
            break;
    }
}

- (void)mouseMoved:(NSEvent*)theEvent {
    double x = theEvent.deltaX;
    double y = theEvent.deltaY;
    
    relativeMouseX += x;
    relativeMouseY += y;
    
//    NSLog(@"%f, %f", relativeMouseX, relativeMouseY);
}


@end
