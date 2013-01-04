//
//  AppDelegate.m
//  IbexMac
//
//  Created by Hesham Wahba on 12/25/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import "AppDelegate.h"

#import "MyOpenGLView.h"

#import "ibex.h"

@implementation AppDelegate


- (void) awakeFromNib {
    CGRect r;
    r.origin = CGPointMake(0,0);
    r.size = NSScreen.mainScreen.frame.size;
    
    physicalWidth = r.size.width;
    physicalHeight = r.size.height;
    
    width = r.size.width;
    height = r.size.height;
    
    [_window setFrame:r display:YES];
    NSLog(@"%@", NSStringFromRect(r));
    
    [_window setStyleMask:NSBorderlessWindowMask];
    [_window setAcceptsMouseMovedEvents:YES];
    [_window setMovableByWindowBackground:YES];
//    [_window setLevel:NSNormalWindowLevel];
    [_window setExcludedFromWindowsMenu:YES];
    [_window setLevel:NSScreenSaverWindowLevel];
//    [_window setLevel:CGShieldingWindowLevel()+1];
        [_window setFrame:NSScreen.mainScreen.frame display:YES];
//    [_window toggleFullScreen:self];
    [_window makeKeyAndOrderFront:nil];
    
//    [_window setContentSize:r.size];
//    [_window setFrameOrigin:NSPointFromCGPoint(CGPointMake(10,20))];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
//    [NSCursor hide];
    CGDisplayHideCursor(kCGDirectMainDisplay);
    
//    CFStringRef propertyString = CFStringCreateWithCString(NULL, "SetsCursorInBackground", kCFStringEncodingMacRoman);
//    CGSSetConnectionProperty(_CGSDefaultConnection(), _CGSDefaultConnection(), propertyString, kCFBooleanTrue);
//    CFRelease(propertyString);
//    CGDisplayHideCursor(kCGDirectMainDisplay);
    
//    [_window setIgnoresMouseEvents:YES];
//    [_window setAcceptsMouseMovedEvents:YES];
    [_window setLevel:NSScreenSaverWindowLevel];
    
    CGRect mainDisplayRect = NSScreen.mainScreen.frame;
    
    
    
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer, 0,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core
    };
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    
    NSRect viewRect = NSMakeRect(0.0, 0.0, mainDisplayRect.size.width, mainDisplayRect.size.height);
    MyOpenGLView *fullScreenView = [[MyOpenGLView alloc] initWithFrame:viewRect pixelFormat: pixelFormat];
    [_window setContentView: fullScreenView];
    [_window makeKeyAndOrderFront:self];
    [_window makeFirstResponder:fullScreenView];
    
    return;
    // Insert code here to initialize your application
    char *text = "Hello, World!";
    CGDirectDisplayID display = kCGDirectMainDisplay; // 1
    
    
    const int MAX_DISPLAYS = 5;
    CGDirectDisplayID displays[MAX_DISPLAYS];
    uint32_t numDisplays;
    
    CGGetActiveDisplayList(MAX_DISPLAYS, displays, &numDisplays);
    display = displays[0];
    display = CGMainDisplayID();
    
            CGRect r = NSScreen.mainScreen.frame;
    
    
    CFArrayRef a = CGWindowListCreate(
                                              //CGWindowListOption option,
                                              kCGWindowListOptionOnScreenOnly,
                                              kCGNullWindowID
                                              //                                                      CGWindowID relativeToWindow
                                      );
    sleep(1);
    
//    CGError err = CGDisplayCapture (display); // 2
    CGError err = CGDisplayCaptureWithOptions(display, kCGCaptureNoFill);
//    CGError err = CGCaptureAllDisplaysWithOptions(kCGCaptureNoFill);
    
    if (err == kCGErrorSuccess)
    {
//        CGSize s = CGDisplayScreenSize (display);

        CGContextRef ctx = CGDisplayGetDrawingContext (display); // 3
        if (ctx != NULL)
        {
            for(int i2 = 0; i2 < 50; ++ i2) {

                CGImageRef i = CGWindowListCreateImageFromArray(
                                                                CGRectInfinite,
                                                                a,
                                                                kCGWindowImageDefault
                                                                );
//                CGImageRef i = CGWindowListCreateImage(CGRectInfinite,
//                                                       kCGWindowListOptionOnScreenOnly,//kCGWindowListOptionAll,
//                                                       kCGNullWindowID,kCGWindowImageDefault);
                
//            CGImageRef i = CGDisplayCreateImage(display);
            CGContextDrawImage(ctx, r, i);
            CGImageRelease(i);
                
                struct timespec tim, tim2;
                tim.tv_sec  = 0;
                tim.tv_nsec = 50000000L;
                nanosleep(&tim,&tim2);
            }
            
//            CGContextSelectFont (ctx, "Times-Roman", 48, kCGEncodingMacRoman);
//            CGContextSetTextDrawingMode (ctx, kCGTextFillStroke);
//            CGContextSetRGBFillColor (ctx, 1, 1, 1, 0.75);
//            CGContextSetRGBStrokeColor (ctx, 1, 1, 1, 0.75);
//            CGContextShowTextAtPoint (ctx, 40, 40, text, strlen(text)); // 4
//            sleep (4); // 5
        }
        CGDisplayRelease (display); // 6
    }
    
//                NSLog(@"%@", r);
}

@end
