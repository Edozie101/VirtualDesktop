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
    CGRect mainDisplayRect = NSScreen.mainScreen.frame;
    
    CGDisplayHideCursor(kCGDirectMainDisplay);
    
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer, 0,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core
    };
    /*NSOpenGLPixelFormat* */pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    
    NSRect viewRect = NSMakeRect(0.0, 0.0, mainDisplayRect.size.width, mainDisplayRect.size.height);
    fullScreenView = [[MyOpenGLView alloc] initWithFrame:viewRect pixelFormat: pixelFormat];
    
    [_window setContentView: fullScreenView];
    [_window makeKeyAndOrderFront:self];
    [_window makeFirstResponder:fullScreenView];
    
    [_window setAcceptsMouseMovedEvents:YES];
    [_window setMovableByWindowBackground:YES];
    [_window setExcludedFromWindowsMenu:YES];
}

@end
