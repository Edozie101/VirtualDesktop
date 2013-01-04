//
//  MyOpenGLView.m
//  IbexMac
//
//  Created by Hesham Wahba on 12/27/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

//#include "opengl_helpers.h"

#import <QuartzCore/QuartzCore.h>

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
    RegisterEventHotKey(keyCode,cmdKey+shiftKey,keyID,GetApplicationEventTarget(),0,&theRef);
}
- (void) unregisterHotkey {
    // release self one more time...
}

NSTimer *t;

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format {
    self = [super initWithFrame:frameRect pixelFormat:format];
    if (self) {
        [self registerHotkey];
        [self controlDesktopUpdate];
    }
    
    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self registerHotkey];
    }
    
    return self;
}
- (id)init
{
    self = [super init];
    if (self) {
        [self registerHotkey];
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        [self registerHotkey];
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

    if(false) {
//    NSLog(@"awakeFromNib");
    renderTimer = [NSTimer timerWithTimeInterval:0.001   //a 1ms time interval
                                           target:self
                                         selector:@selector(refresh)
                                         userInfo:nil
                                          repeats:YES];
                   
                   [[NSRunLoop currentRunLoop] addTimer:renderTimer
                                                forMode:NSDefaultRunLoopMode];
                   [[NSRunLoop currentRunLoop] addTimer:renderTimer
                                                forMode:NSEventTrackingRunLoopMode]; //Ensure timer fires during resize
    }
    
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
- (void)createGLTexture:(GLuint *)texName fromCGImage:(CGImageRef)img
{
	GLubyte *spriteData = NULL;
	CGContextRef spriteContext;
	GLuint imgW, imgH, texW, texH;
	
	imgW = CGImageGetWidth(img);
	imgH = CGImageGetHeight(img);
	
	// Find smallest possible powers of 2 for our texture dimensions
	for (texW = 1; texW < imgW; texW *= 2) ;
	for (texH = 1; texH < imgH; texH *= 2) ;
    texW = imgW;
    texH = imgH;
	
	// Allocated memory needed for the bitmap context
	spriteData = (GLubyte *) calloc(texH, texW * 4);
	// Uses the bitmatp creation function provided by the Core Graphics framework.
	spriteContext = CGBitmapContextCreate(spriteData, texW, texH, 8, texW * 4, CGImageGetColorSpace(img), kCGImageAlphaPremultipliedLast);
	
	// Translate and scale the context to draw the image upside-down (conflict in flipped-ness between GL textures and CG contexts)
	CGContextTranslateCTM(spriteContext, 0., texH);
	CGContextScaleCTM(spriteContext, 1., -1.);
	
	// After you create the context, you can draw the sprite image to the context.
	CGContextDrawImage(spriteContext, CGRectMake(0.0, 0.0, imgW, imgH), img);
	// You don't need the context at this point, so you need to release it to avoid memory leaks.
	CGContextRelease(spriteContext);
	
    glEnable(GL_TEXTURE_2D);
	// Use OpenGL ES to generate a name for the texture.
	glGenTextures(1, texName);
	// Bind the texture name.
	glBindTexture(GL_TEXTURE_2D, *texName);
	// Specify a 2D texture image, providing the a pointer to the image data in memory
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, spriteData);
	// Set the texture parameters to use a minifying filter and a linear filer (weighted average)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// Enable use of the texture
//	glEnable(GL_TEXTURE_2D);
	// Set a blending function to use
//	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	// Enable blending
//	glEnable(GL_BLEND);
	
	free(spriteData);
}

- (void) refresh {
    NSLog(@"REFRESH");
    [self setNeedsDisplay:YES];
}
static void drawAnObject (GLuint texture, GLuint cursor, const CGPoint cursorPos)
{
    glDisable(GL_BLEND);
    
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, texture );
    
    glColor4f(1.0f, 1, 1, 1.0);
    glBegin(GL_TRIANGLES);
    {
        glTexCoord2d(0, 0);
        glVertex3f(  0.0,  0, 0.0);
        
        glTexCoord2d(1, 0);
        glVertex3f( 0.5, 0, 0.0);
        
        glTexCoord2d(1, 1);
        glVertex3f(  0.5, 1 ,0.0);
        
        glTexCoord2d(0, 0);
        glVertex3f(  0.0,  0, 0.0);
        
        glTexCoord2d(1, 1);
        glVertex3f( 0.5, 1, 0.0);
        
        glTexCoord2d(0, 1);
        glVertex3f(  0, 1 ,0.0);
        
        
        
        glTexCoord2d(0, 0);
        glVertex3f(  0.5,  0, 0.0);
        
        glTexCoord2d(1, 0);
        glVertex3f( 1, 0, 0.0);
        
        glTexCoord2d(1, 1);
        glVertex3f(  1, 1 ,0.0);
        
        glTexCoord2d(0, 0);
        glVertex3f(  0.5,  0, 0.0);
        
        glTexCoord2d(1, 1);
        glVertex3f( 1, 1, 0.0);
        
        glTexCoord2d(0, 1);
        glVertex3f(  0.5, 1 ,0.0);
        
        
    }
    glEnd();
    
    
    
//    glDisable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture( GL_TEXTURE_2D, cursor );
    glColor4f(1, 1, 1, 1.0);
    glBegin(GL_TRIANGLES);
    {
        double x0, x1, y0, y1, z;
        x0 = 0.+(cursorPos.x/1440.0)/2.0;
        x1 = 0.+((cursorPos.x+20.0)/1440.0)/2.0;
        y0 = 0.+(cursorPos.y+0.)/900.0;
        y1 = 0.+(cursorPos.y-20.)/900.0;
        z = 0.5;
        glTexCoord2d(0, 0);
        glVertex3f(  x0,  y0, z);
        
        glTexCoord2d(1, 0);
        glVertex3f( x1, y0, z);
        
        glTexCoord2d(1, -1);
        glVertex3f(  x1, y1 ,z);
        
        glTexCoord2d(0, 0);
        glVertex3f(  x0,  y0, z);
        
        glTexCoord2d(1, -1);
        glVertex3f( x1, y1, z);
        
        glTexCoord2d(0, -1);
        glVertex3f(  x0, y1,z);
        
        
        
        
        
        
        x0 = 0.5+(cursorPos.x/1440.0)/2.0;
        x1 = 0.5+((cursorPos.x+20.0)/1440.0)/2.0;
        y0 = 0.+(cursorPos.y+0.)/900.0;
        y1 = 0.+(cursorPos.y-20.)/900.0;
        z = 0.5;
        glTexCoord2d(0, 0);
        glVertex3f(  x0,  y0, z);
        
        glTexCoord2d(1, 0);
        glVertex3f( x1, y0, z);
        
        glTexCoord2d(1, -1);
        glVertex3f(  x1, y1 ,z);
        
        glTexCoord2d(0, 0);
        glVertex3f(  x0,  y0, z);
        
        glTexCoord2d(1, -1);
        glVertex3f( x1, y1, z);
        
        glTexCoord2d(0, -1);
        glVertex3f(  x0, y1,z);
    }
    glEnd();
    
    glDeleteTextures( 1, &texture );
}

//GLuint cursor(0);
CGPoint cursorPos;
- (GLuint)getScreenshot {
    if(desktopTexture) {
        glDeleteTextures( 1, &desktopTexture);
    }
    if(cursor) {
        glDeleteTextures( 1, &cursor );
    }

    cursorPos = NSEvent.mouseLocation;
    CGPoint hotSpot = NSCursor.currentSystemCursor.hotSpot;
    cursorPos.x -= hotSpot.x;
    cursorPos.y += hotSpot.y;
    [self createGLTexture:&cursor fromCGImage:[NSCursor.currentSystemCursor.image CGImageForProposedRect:nil context:nil hints:nil]];
    
    CFArrayRef a = CGWindowListCreate(
                                      kCGWindowListOptionOnScreenBelowWindow,
                                      (CGWindowID)_window.windowNumber
                                      );
    
    CGImageRef i = CGWindowListCreateImageFromArray(
                                                    CGRectInfinite,
                                                    a,
                                                    kCGWindowImageDefault
                                                    );

    [self createGLTexture:&desktopTexture fromCGImage:i];
    CGImageRelease(i);
    return desktopTexture;
}

- (void)drawRect:(NSRect)dirtyRect
{
    
//    if(ibex == nil) {
//        ibex = new Ibex(0,nil);
//    }
//    
////    NSLog(@"Start");
//    static NSOpenGLContext* context;
//    if(context == nil)
//        context = [self openGLContext];
//    // Drawing code here.
//    [context makeCurrentContext];
//    
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(0, 1, 0, 1, -10, 10);
//    glMatrixMode(GL_MODELVIEW);
//    
//    glEnable(GL_DEPTH_TEST);
//
//    glClearColor(0, 0, 0, 0);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    
//    desktopTexture = [self getScreenshot];
//    drawAnObject(desktopTexture, cursor, cursorPos);
//    
//    ibex->render();
//    
//    glFlush();
//    
//    [context flushBuffer];
//    
//    [self setNeedsDisplay:YES];
    
    NSLog(@"Here");
}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)time
{
//    NSLog(@"getFrameForTime start");
    GLfloat timeDiff = (GLfloat)(time->videoTime) / (GLfloat)(time->videoTimeScale);
    
    //    NSLog(@"Start");
    static NSOpenGLContext* context;
    if(context == nil)
        context = [self openGLContext];
    // Drawing code here.
    [context makeCurrentContext];
    
    // Add your drawing codes here
    if(ibex == nil) {
        ibex = new Ibex(0,nil);
    }
    
    if(false) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, 1, 0, 1, -10, 10);
        glMatrixMode(GL_MODELVIEW);
        
        glEnable(GL_DEPTH_TEST);
        
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
//    desktopTexture =
    [self getScreenshot];
    cursorPosX = cursorPos.x;
    cursorPosY = cursorPos.y;
//    drawAnObject(desktopTexture, cursor, cursorPos);
//    [context flushBuffer];
//    return kCVReturnSuccess;
    
    ibex->render(timeDiff);
    
//    glFlush();
    
    [context flushBuffer];
//    checkForErrors();
    
//    NSLog(@"getFrameForTime end");
    
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
