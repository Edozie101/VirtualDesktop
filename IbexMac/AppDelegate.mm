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
#import <IOKit/graphics/IOGraphicsLib.h>

@interface NSScreen (DisplayName)
- (NSString *)displayName;
@end

@implementation NSScreen (DisplayName)
- (NSString *)displayName {
    NSString *screenName = nil;
    
    io_service_t framebuffer = CGDisplayIOServicePort([[[self deviceDescription] objectForKey:@"NSScreenNumber"] unsignedIntValue]);
    NSDictionary *deviceInfo = (__bridge NSDictionary *)IODisplayCreateInfoDictionary(framebuffer, kIODisplayOnlyPreferredName);
    NSDictionary *localizedNames = [deviceInfo objectForKey:[NSString stringWithUTF8String:kDisplayProductName]];
    
    if ([localizedNames count] > 0) {
        screenName = [localizedNames objectForKey:[[localizedNames allKeys] objectAtIndex:0]];
    }
    else {
        screenName = @"Unknown";
    }
    
    return screenName;
}
@end

@implementation AppDelegate

@synthesize pixelFormat;

- (NSRect) getRiftDisplay {
    NSArray *screenArray = [NSScreen screens];
    for(NSScreen *screen in screenArray) {
        NSDictionary *screenDescription = [screen deviceDescription];
        NSLog(@"Device ID: %@", screenDescription);
        NSLog(@"Device Name: %@", [screen displayName]);
        if([[screen displayName] rangeOfString:@"Rift"].location != NSNotFound) {
            return screen.frame;
        }
    }
    return CGRectNull;
}
- (NSScreen*) getRiftScreen {
    NSArray *screenArray = [NSScreen screens];
    for(NSScreen *screen in screenArray) {
        NSDictionary *screenDescription = [screen deviceDescription];
        NSLog(@"Device ID: %@", screenDescription);
        NSLog(@"Device Name: %@", [screen displayName]);
        if([[screen displayName] rangeOfString:@"Rift"].location != NSNotFound) {
            return screen;
        }
    }
    return nil;
}
- (void) awakeFromNib {
    CGRect r;
    r.origin = CGPointMake(0,0);
    r.size = NSScreen.mainScreen.frame.size;
    
    physicalWidth = r.size.width;
    physicalHeight = r.size.height;
    
    width = physicalWidth;
    height = physicalHeight;
    
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
    
    const CGRect rift = [self getRiftDisplay];
    if(!CGRectIsNull(rift)) {
        width = rift.size.width;
        height = rift.size.height;
        
        windowWidth = width;
        windowHeight = height;
        textureWidth = width*2;
        textureHeight = height*2;
        
        [_window setFrame:rift display:YES];
    }
}

@end
