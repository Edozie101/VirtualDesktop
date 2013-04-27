//
//  MyOgreView.m
//  IbexMac
//
//  Created by Hesham Wahba on 3/19/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef GLX_GLXEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#endif

#import "MyOgreView.h"
#import "ScreenshotView.h"

#include "ibex.h"
#include "sixense_controller.h"

#include <ApplicationServices/ApplicationServices.h>
#import <QuartzCore/QuartzCore.h>

#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>

#import <CoreVideo/CVDisplayLink.h>

#import <mach/mach_time.h>
#import <libkern/OSAtomic.h>

#import <Carbon/Carbon.h>
#import <OpenGL/glext.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/CGLTypes.h>
#import <OpenGL/CGLCurrent.h>

#import "MyOpenGLView.h"

#include <OgreRoot.h>
#include <OgreRenderSystem.h>
#include <OgreGLRenderSystem.h>
#include <OSX/OgreOSXCocoaContext.h>

#include <iostream>

@implementation MyOgreView

static Ibex *ibex = nil;

static EventHandlerUPP hotKeyFunction;

//static NSCondition *cocoaCondition;

// ---------------------------------------------------------------------------
// Function: checkForErrors
// Design:   Belongs to OpenGL component
// Purpose:  Prints OpenGL errors
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
inline static bool checkForErrors()
{
    static bool doCheck = true;
    
    if (!doCheck)
        return false;
    
    const char* errorString = 0;
    bool retVal = false;
    GLenum error = glGetError();
    switch(error) {
        case GL_NO_ERROR:
            retVal = true;
            break;
            
        case GL_INVALID_ENUM:
            errorString = "GL_INVALID_ENUM";
            break;
            
        case GL_INVALID_VALUE:
            errorString = "GL_INVALID_VALUE";
            break;
            
        case GL_INVALID_OPERATION:
            errorString = "GL_INVALID_OPERATION";
            break;
            
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
            
            // OpenGLES Specific Errors
#ifdef ATHENA_OPENGLES
        case GL_STACK_OVERFLOW:
            errorString = "GL_STACK_OVERFLOW";
            break;
            
        case GL_STACK_UNDERFLOW:
            errorString = "GL_STACK_UNDERFLOW";
            break;
#endif
            
        case GL_OUT_OF_MEMORY:
            errorString = "GL_OUT_OF_MEMORY";
            break;
            
        default:
            errorString = "UNKNOWN";
            break;
    }
    
    if (!retVal)
        std::cerr << "OpenGL ERROR: " << errorString << " -- " << error << std::endl;
    
    return retVal;
}

// hides cursor in the background!
extern "C" void CGSSetConnectionProperty(int, int, CFStringRef, CFBooleanRef);
extern "C" int _CGSDefaultConnection();
- (void)hideCursor {
    CFStringRef propertyString;
    
    // Hack to make background cursor setting work
    propertyString = CFStringCreateWithCString(NULL, "SetsCursorInBackground", kCFStringEncodingUTF8);
    CGSSetConnectionProperty(_CGSDefaultConnection(), _CGSDefaultConnection(), propertyString, kCFBooleanTrue);
    CFRelease(propertyString);
    // Hide the cursor and wait
    CGDisplayHideCursor(kCGDirectMainDisplay);
}
- (void)controlDesktopUpdate {
    [self hideCursor];
    
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
static OSStatus hotKeyHandler(EventHandlerCallRef nextHandler,EventRef theEvent, void *userData)
{
    MyOgreView *obj =  (__bridge MyOgreView*)userData;
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

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        myInitSixense();
        
        window = (unsigned long)self;
    }
    
    return self;
}



// This is the renderer output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    CVReturn result = [((__bridge MyOgreView*)displayLinkContext) getFrameForTime:outputTime];
    return result;
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
        case kVK_ANSI_R:
            resetPosition = 1;
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
    relativeMouseX += theEvent.deltaX;
    relativeMouseY += theEvent.deltaY;
    
//    NSLog(@"%f, %f", relativeMouseX, relativeMouseY);
}

- (void)createGLTexture:(GLuint *)texName fromCGImage:(CGImageRef)img andDataCache:(GLubyte**)spriteData andClear:(bool)clear
{
    bool newTexture = (*texName == 0);
	CGContextRef spriteContext;
	GLuint imgW, imgH, texW, texH;
    
	imgW = CGImageGetWidth(img);
	imgH = CGImageGetHeight(img);
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
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)texW);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if(newTexture) {
        // Use OpenGL ES to generate a name for the texture.
        GLuint t;
        glGenTextures(1, &t);
        *texName = t;
        
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

- (void)createGLTextureNoAlpha:(GLuint *)texName fromCGImage:(CGImageRef)img andDataCache:(GLubyte**)spriteData andClear:(bool)clear
{
    bool newTexture = (*texName == 0);
	CGContextRef spriteContext;
	GLuint imgW, imgH, texW, texH;
    
	imgW = CGImageGetWidth(img);
	imgH = CGImageGetHeight(img);
    texW = imgW;
    texH = imgH;
	
    if(*spriteData == NULL) {
        // Allocated memory needed for the bitmap context
        *spriteData = (GLubyte*) calloc(texH, texW * 4);
        NSLog(@"Allocating more memory");
    }
    
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
	// You don't need the context at this point, so you need to release it to avoid memory leaks.
	CGContextRelease(spriteContext);
	
    glEnable(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)texW);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if(newTexture) {
        // Use OpenGL ES to generate a name for the texture.
        GLuint t;
        glGenTextures(1, &t);
        *texName = t;
        checkForErrors();
        
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

static NSOpenGLContext *currentContext = nil;
static NSOpenGLPixelFormat *pixelFormat = nil;
static NSOpenGLContext* newContext = nil;
////static bool done = 0;
//- (void)loopScreenshot {
//    newContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:currentContext];
//    
//    static GLubyte *cursorData = NULL;
//    static GLubyte *s = NULL;
//    
//    static NSCursor *systemCursor;
//    static dispatch_once_t onceToken;
//    dispatch_once(&onceToken, ^{
//        systemCursor = NSCursor.currentSystemCursor;
//    });
//    [newContext makeCurrentContext];
//    
//    while(1) {
//        done = 0;
//        [newContext makeCurrentContext];
//        systemCursor = [NSCursor currentSystemCursor];
//        if(systemCursor != nil) {
//            NSImage *cursorImage = systemCursor.image;
//            CGImageRef cursorImageRef = [cursorImage CGImageForProposedRect:nil context:nil hints:nil];
//            [self createGLTexture:&cursor fromCGImage:cursorImageRef andDataCache:&cursorData andClear:YES];
//        }
//        CFArrayRef a = CGWindowListCreate(
//                                          kCGWindowListOptionOnScreenBelowWindow,
//                                          (CGWindowID)_window.windowNumber
//                                          );
//        
//        CGImageRef img = CGWindowListCreateImageFromArray(
//                                                          CGRectInfinite,
//                                                          a,
//                                                          kCGWindowImageDefault
//                                                          );
//        CFRelease(a);
//        
//        [self createGLTextureNoAlpha:&desktopTexture fromCGImage:img andDataCache:&s andClear:NO];
//        
//        glFlush();
//        
//        CGImageRelease(img);
//        
//        [cocoaCondition lock];
//        [cocoaCondition wait];
//        [cocoaCondition unlock];
//    }
//}
static CGPoint cursorPos;
- (GLuint)getScreenshot {
    {
        static NSCursor *cursor;
        cursor = [NSCursor currentSystemCursor];
        
        cursorPos = NSEvent.mouseLocation;
        CGPoint hotSpot = cursor.hotSpot;
        cursorPos.x -= hotSpot.x;
        cursorPos.y += hotSpot.y;
    }
    
    return desktopTexture;
}

- (void)drawRect:(NSRect)dirtyRect
{
    static bool init = false;
    if(!init) {
        init = true;
        
        cocoaCondition = [[NSCondition alloc] init];
        
        [self registerHotkey];
        [self controlDesktopUpdate];

        strcpy(mResourcePath, [[[NSBundle mainBundle] resourcePath]
                               cStringUsingEncoding:NSUTF8StringEncoding]);
        
        // Create a display link capable of being used with all active displays
        CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
        
        // Set the renderer output callback function
        CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, (void*)CFBridgingRetain(self));
        
        CGDirectDisplayID theCGDisplayID = 0;
        CVDisplayLinkSetCurrentCGDisplay(displayLink, theCGDisplayID);
        
        CVDisplayLinkStart(displayLink);
    }
}

MyOgreView *myOgreView = nil;
void startDesktopCapture(void *c, void *p) {
//    Ogre::GLRenderSystem *rs = static_cast<Ogre::GLRenderSystem*>(Ogre::Root::getSingleton().getRenderSystem());
    Ogre::GLRenderSystem *rs = (Ogre::GLRenderSystem*)(Ogre::Root::getSingleton().getRenderSystem());
    Ogre::OSXCocoaContext *mainContext = (Ogre::OSXCocoaContext*)rs->_getMainContext();
    NSOpenGLContext *shareContext = mainContext == 0 ? 0 : mainContext->getContext();
    NSOpenGLPixelFormat *mGLPixelFormat = mainContext == 0 ? 0 : mainContext->getPixelFormat();
    currentContext = shareContext;
    pixelFormat = mGLPixelFormat;
    
//    currentContext = [NSOpenGLContext currentContext];
    
//    CGLContextObj* contextObj = (CGLContextObj*)currentContext.CGLContextObj;
//    CGLPixelFormatObj pixelFormatObj = CGLGetPixelFormat(*contextObj);
//    pixelFormat = [[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:(void*)&pixelFormatObj];
    
//    [myOgreView performSelectorInBackground:@selector(loopScreenshot) withObject:nil];
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        glFlush();
        @autoreleasepool {
            //[self performSelectorInBackground:@selector(loopScreenshot) withObject:nil];
            myOgreView.screenshotView.pixelFormat = pixelFormat;
            myOgreView.screenshotView.share = currentContext;
            [myOgreView.screenshotView performSelectorInBackground:@selector(loopScreenshot) withObject:nil];
        }
    });
    
//    [myOgreView loopScreenshot];
}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)time
{
    if(ibex == nil) {
        myOgreView = self;
        
        currentContext = [NSOpenGLContext currentContext];
        
        ibex = new Ibex(0,nil);
        ibex->render(0);
        
//        currentContext = [NSOpenGLContext currentContext];
//        
//        CGLContextObj* contextObj = (CGLContextObj*)currentContext.CGLContextObj;
//        CGLPixelFormatObj pixelFormatObj = CGLGetPixelFormat(*contextObj);
//        pixelFormat = [[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:(void*)&pixelFormatObj];
//
//        [self performSelectorInBackground:@selector(loopScreenshot) withObject:nil];
    }
    
    CGDisplayHideCursor(kCGDirectMainDisplay);
    
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
    
    [self getScreenshot];
    cursorPosX = cursorPos.x;
    cursorPosY = cursorPos.y;
    ibex->render(timeDiff);
    
    done = 1;
    
    [cocoaCondition lock];
    [cocoaCondition signal];
    [cocoaCondition unlock];
    
    return kCVReturnSuccess;
}

@end
