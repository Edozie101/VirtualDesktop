//
//  MyOpenGLView.m
//  IbexMac
//
//  Created by Hesham Wahba on 12/27/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import "opengl_helpers.h"

#import "video/IbexVideoPlayer.h"

//#include "opengl_helpers.h"
#include <iostream>

#import <QuartzCore/QuartzCore.h>

#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>

#import <CoreVideo/CVDisplayLink.h>

#import <mach/mach_time.h>
#import <libkern/OSAtomic.h>

//#import <OpenGL/glext.h>
#import "MyOpenGLView.h"

#import <Carbon/Carbon.h>

#include "ibex.h"

//#import <OpenGL/OpenGL.h>

#include <ApplicationServices/ApplicationServices.h>

#import "ScreenshotView.h"

#include "sixense/sixense_controller.h"

#include "windows/Window.h"

#include "RendererPlugin.h"

bool doubleBuffered = true;
char mResourcePath[1024];

@implementation MyOpenGLView

static Ibex::Ibex *ibex = nil;

static EventHandlerUPP hotKeyFunction;

NSCondition *cocoaCondition;

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
    MyOpenGLView *obj =  (__bridge MyOpenGLView*)userData;
    if(!controlDesktop) {
        CGDisplayMoveCursorToPoint(kCGDirectMainDisplay, CGPointMake(cursorPosX, physicalHeight-cursorPosY));
    }
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
    
    keyCode = kVK_ANSI_G;//kVK_ANSI_R; //F19
    theRef = NULL;
    keyID.signature = 'FOO '; //arbitrary string
    keyID.id = 1;
    RegisterEventHotKey(keyCode, controlKey|shiftKey, keyID,GetApplicationEventTarget(), 0, &theRef);
}
- (void) unregisterHotkey {
    // release self one more time...
}

static NSTimer *t;

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format {
    self = [super initWithFrame:frameRect pixelFormat:format];
    if (self) {
#if _USE_SIXENSE
        myInitSixense();
#endif
        
        cocoaCondition = [[NSCondition alloc] init];
        
        _ibexVideoPlayer = [[IbexVideoPlayer alloc] init];
        
        [self registerHotkey];
        [self controlDesktopUpdate];
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
    static bool init = false;
    if(init) {
        return;
    }
    init = true;
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

static int desktopTextureIndex = 0;
static GLuint desktopTextures[2] = {NULL, NULL};
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

    [newContext makeCurrentContext];
    while(1) {
            [self createGLTexture:&cursor fromCGImage:[NSCursor.currentSystemCursor.image CGImageForProposedRect:nil context:nil hints:nil] andDataCache:&cursorData andClear:YES];
            
            
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
            
            [self createGLTextureNoAlpha:&desktopTextures[index] fromCGImage:img andDataCache:((index)?&s2:&s) andClear:NO];
            glFlush();
            
//            [newContext flushBuffer];            
            
            CGImageRelease(img);
            
            
            desktopTextureIndex = index;
            done = 1;
            index = (index+1)%2;
//            NSLog(@"drawing desktop done");
    }
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, *spriteData);
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


- (void)createGLTexture:(GLuint *)texName fromCGImage:(CGImageRef)img andDataCache:(GLubyte**)spriteData andClear:(bool)clear
{
    bool newTexture = (*texName == 0);
	CGContextRef spriteContext;
	GLuint imgW, imgH, texW, texH;

	imgW = CGImageGetWidth(img);
	imgH = CGImageGetHeight(img);
    texW = imgW;
    texH = imgH;
    
    cursorSize = imgH;
	
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, *spriteData);
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

static CGPoint cursorPos;
- (GLuint)getScreenshot {
    {
        cursorPos = NSEvent.mouseLocation;
        CGPoint hotSpot = NSCursor.currentSystemCursor.hotSpot;
        cursorPos.x -= hotSpot.x;
        cursorPos.y += hotSpot.y;
    }
    
    return desktopTexture;
}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)time
{
//    return kCVReturnSuccess;
    //    timeToDoWork++;
    
//    [self hideCursor];
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
        sprintf(fpsString, "FPS: %4.2f", ((double)frame)/fpsTime);
        frame = 0;
        fpsTime = 0;
    }

#if _USE_SIXENSE
    mySixenseRefresh();
#endif
    
    //    NSLog(@"Start");
    static NSOpenGLContext* glContext;
    if(glContext == nil)
        glContext = [self openGLContext];
    
    [glContext makeCurrentContext];
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        glFlush();
        @autoreleasepool {
            _screenshotView.pixelFormat = self.pixelFormat;
            _screenshotView.share = self.openGLContext;
            [_screenshotView performSelectorInBackground:@selector(loopScreenshot) withObject:nil];
        }
    });
    
    static dispatch_once_t onceToken2;
    dispatch_once(&onceToken2, ^{
        _ibexVideoPlayer.pixelFormat = self.pixelFormat;
        _ibexVideoPlayer.share = self.openGLContext;
//        [_ibexVideoPlayer performSelectorInBackground:@selector(loadVideo:andIsStereo:) withObject:@[<movieFilePath>,@false]];
    });
    if(ibex != nil && (ibex->renderer->window.getSelectedVideo() || ibex->renderer->window.getSelectedCamera())) {
        if(ibex->renderer->window.getSelectedVideo()) {
            NSString *videoPath = [NSString stringWithUTF8String:ibex->renderer->window.getSelectedVideoPath().c_str()];
            [_ibexVideoPlayer performSelectorInBackground:@selector(loadVideo:andIsStereo:) withObject:@[videoPath,(ibex->renderer->window.getIsStereoVideo())?@YES : @NO]];
        } else {
            NSNumber *cameraID = [NSNumber numberWithInt:ibex->renderer->window.getSelectedCameraID()];
            [_ibexVideoPlayer performSelectorInBackground:@selector(loadCamera:andIsStereo:) withObject:@[cameraID,(ibex->renderer->window.getIsStereoVideo())?@YES : @NO]];
        }
        ibex->renderer->window.setSelectedVideo(false);
        ibex->renderer->window.setSelectedCamera(false);
    }
    
    videoWidth = [_ibexVideoPlayer width];
    videoHeight = [_ibexVideoPlayer height];
    
    [self getScreenshot];
    if(controlDesktop) {
        cursorPosX = cursorPos.x;
        cursorPosY = cursorPos.y;
    }
    
    if(ibex == nil) {
        ibex = new Ibex::Ibex(0,nil);
    }
    
    videoTexture[0] = _ibexVideoPlayer.videoTexture[0];
    videoTexture[1] = _ibexVideoPlayer.videoTexture[1];
    ibex->render(timeDiff);
    
//    glFlush();
    
    [glContext flushBuffer];
    done = 1;
    
    [cocoaCondition lock];
    [cocoaCondition signal];
    [cocoaCondition unlock];
    
//    checkForErrors();
    
    return kCVReturnSuccess;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyDown:(NSEvent *)theEvent {
    int processed = 0;
    if(showDialog) {
        processed = ibex->renderer->window.processKey(theEvent.keyCode, 1);
    }
    if(!processed) {
        switch(theEvent.keyCode) {
            case kVK_UpArrow:
            case kVK_ANSI_W:
                walkForward = 1;
                break;
            case kVK_DownArrow:
            case kVK_ANSI_S:
                walkForward = -1;
                break;
            case kVK_LeftArrow:
            case kVK_ANSI_A:
                strafeRight = -1;
                break;
            case kVK_RightArrow:
            case kVK_ANSI_D:
                strafeRight = 1;
                break;
            case kVK_Space:
                break;
            case kVK_ANSI_Q:
                displayShape = (displayShape == FlatDisplay) ? SphericalDisplay : (displayShape == SphericalDisplay) ? CylindricalDisplay: FlatDisplay;
                break;
            case kVK_ANSI_P:
                SBS = !SBS;
                break;
            case kVK_ANSI_Slash:
                if(!controlDesktop) {
                    showDialog = !showDialog;
                }
                break;
            case kVK_ANSI_Minus:
                IOD -= 0.0005;
                lensParametersChanged = true;
                break;
            case kVK_ANSI_Equal:
                IOD += 0.0005;
                lensParametersChanged = true;
                break;
        }
    }
}
- (void)keyUp:(NSEvent *)theEvent {
    int processed = 0;
    if(showDialog) {
        processed = ibex->renderer->window.processKey(theEvent.keyCode, 0);
    }
    if(!processed) {
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
            case kVK_UpArrow:
            case kVK_ANSI_W:
            case kVK_DownArrow:
            case kVK_ANSI_S:
                walkForward = 0;
                break;
            case kVK_LeftArrow:
            case kVK_ANSI_A:
            case kVK_RightArrow:
            case kVK_ANSI_D:
                strafeRight = 0;
                break;
            case kVK_Space:
                break;
        }
    }
}

- (void)mouseMoved:(NSEvent*)theEvent {
    relativeMouseX += theEvent.deltaX;
    relativeMouseY += theEvent.deltaY;
    
//    NSLog(@"%f, %f", relativeMouseX, relativeMouseY);
}


@end
