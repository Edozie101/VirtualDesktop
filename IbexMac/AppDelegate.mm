//
//  AppDelegate.m
//  IbexMac
//
//  Created by Hesham Wahba on 12/25/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import "AppDelegate.h"

#import "MyOpenGLView.h"

#ifdef ENABLE_OGRE3D
#import "MyOgreView.h"
#endif

#import "ibex.h"

#import "ServerController.h"

@implementation AppDelegate

@synthesize pixelFormat;

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
    [_window setLevel:NSScreenSaverWindowLevel];
    [_window setFrame:NSScreen.mainScreen.frame display:YES];
    
    serverController = [[ServerController alloc] init];
    [serverController startService];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    strcpy(mResourcePath, [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:NSUTF8StringEncoding]);
    
    CGRect mainDisplayRect = NSScreen.mainScreen.frame;
    
    CGDisplayHideCursor(kCGDirectMainDisplay);
    
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer, 0,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core
    };
    /*NSOpenGLPixelFormat* */pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    
    NSRect viewRect = NSMakeRect(0.0, 0.0, mainDisplayRect.size.width, mainDisplayRect.size.height);
#ifdef ENABLE_OGRE3D
    fullScreenView = [[MyOgreView alloc] initWithFrame:viewRect];
#else
    fullScreenView = [[MyOpenGLView alloc] initWithFrame:viewRect pixelFormat: pixelFormat];
#endif
    
    [_window setContentView: fullScreenView];
    [_window makeKeyAndOrderFront:self];
    [_window makeFirstResponder:fullScreenView];
    
    [_window setAcceptsMouseMovedEvents:YES];
    [_window setMovableByWindowBackground:YES];
    [_window setExcludedFromWindowsMenu:YES];
}

@end
