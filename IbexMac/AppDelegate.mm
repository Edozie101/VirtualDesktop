//
//  AppDelegate.m
//  IbexMac
//
//  Created by Hesham Wahba on 12/25/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import "AppDelegate.h"

#import "MyOpenGLView.h"
#import "ScreenshotView.h"
#import "MyWindow.h"

#ifdef ENABLE_OGRE3D
#import "MyOgreView.h"
#endif

#import "ServerController.h"
#import <IOKit/graphics/IOGraphicsLib.h>

#include "oculus/Rift.h"
#undef new
#undef delete

#include "preferences/PreferencesWindowController.h"

bool modifiedDesktop(false);

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
    
    //physicalWidth = r.size.width;
    //physicalHeight = r.size.height;
    
    width = r.size.width; //physicalWidth;
    height = r.size.height; //physicalHeight;
    windowWidth = width;
    windowHeight = height;
    
    CGRect rift = [self getRiftDisplay];
    if(!CGRectIsNull(rift)) {
        width = rift.size.width;
        height = rift.size.height;
        
        windowWidth = width;
        windowHeight = height;
        textureWidth = width*1.4;
        textureHeight = height*1.4;
    } else {
        rift = r;
    }
    
    serverController = [[ServerController alloc] init];
    [serverController startService];
}

- (void)makeWindowTopLevel:(NSWindow*)window_ {
    [window_ setCollectionBehavior:NSWindowCollectionBehaviorMoveToActiveSpace];
    [window_ setExcludedFromWindowsMenu:YES];
    [window_ setMovableByWindowBackground:YES];
    [window_ setExcludedFromWindowsMenu:YES];
    [window_ setStyleMask:NSBorderlessWindowMask];
    [window_ setLevel:NSScreenSaverWindowLevel];
    [window_ makeKeyAndOrderFront:self];
    [window_ setOrderedIndex:0];
    [NSApp activateIgnoringOtherApps:YES];
    [[NSRunningApplication currentApplication] activateWithOptions:NSApplicationActivateAllWindows];
    [fullScreenView controlDesktopUpdate];
}
- (void)makeWindowRegular:(NSWindow*)window_ {
    [window_ setCollectionBehavior:NSWindowCollectionBehaviorMoveToActiveSpace];
    [window_ setExcludedFromWindowsMenu:YES];
    [window_ setMovableByWindowBackground:YES];
    [window_ setExcludedFromWindowsMenu:YES];
    [window_ setStyleMask:NSBorderlessWindowMask];
    [window_ setLevel:NSNormalWindowLevel];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context {
    if([keyPath isEqualToString:@"isTerminated"] && [object isTerminated]) {
        [fullScreenView resumeRendering];
        
        // can't check to see if registered as observer for KVO!
        @try {
            [launchedApplication removeObserver:self forKeyPath:NSStringFromSelector(@selector(isTerminated))];
        } @catch (NSException * __unused exception) {}
        
        launchedApplication = nil;
        [self makeWindowTopLevel:mainWindow];
    }
}

- (void)launchApplication:(NSString*)applicationPath {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
        if(launchedApplication != nil) {
            [launchedApplication removeObserver:self forKeyPath:NSStringFromSelector(@selector(isTerminated))];
            launchedApplication = nil;
            [self makeWindowTopLevel:mainWindow];
        }
        
        if([[NSWorkspace sharedWorkspace] launchApplication:applicationPath] == NO) {
            [self makeWindowTopLevel:mainWindow];
            return;
        }
        NSURL *appURL = [[NSURL alloc] initFileURLWithPath:applicationPath];
        if(appURL == nil) {
            return;
        }
        launchedApplication = [[NSWorkspace sharedWorkspace] launchApplicationAtURL:appURL options:NSWorkspaceLaunchDefault configuration:nil error:nil];
        if(launchedApplication == nil) {
            [self makeWindowTopLevel:mainWindow];
            return;
        }
        [fullScreenView pauseRendering];
        [self makeWindowRegular:mainWindow];
        [launchedApplication addObserver:self
                              forKeyPath:@"isTerminated"
                                 options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld | NSKeyValueObservingOptionInitial)
                                 context:NULL];
    });
}
void launchApplication(const std::string &applicationPath) {
    AppDelegate *appDelegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
    [appDelegate launchApplication:@(applicationPath.c_str())];
}

NSWindow* myWindow;
NSWindow *mainWindow;
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"ApplePersistenceIgnoreState"];

    strcpy(mResourcePath, [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:NSUTF8StringEncoding]);
    
    CGRect mainDisplayRect = NSScreen.mainScreen.frame;
    
    ScreenshotView *screenshotView = [[ScreenshotView alloc] initWithFrame:mainDisplayRect];
    [_window setFrame:mainDisplayRect display:YES];
    [_window setContentView:screenshotView];
    [_window setExcludedFromWindowsMenu:YES];
    [_window setMovableByWindowBackground:YES];
    [_window setExcludedFromWindowsMenu:YES];
    [_window setStyleMask:NSBorderlessWindowMask];
    [_window setBackgroundColor:NSColor.blueColor];
    [_window setLevel:NSScreenSaverWindowLevel];
    [_window setFrame:CGRectMake(0,0,0,0) display:YES];
    [_window makeKeyAndOrderFront:self];
    
    CGDisplayHideCursor(kCGDirectMainDisplay);
    
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core
    };
    /*NSOpenGLPixelFormat* */pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    
    NSRect viewRect = NSMakeRect(0.0, 0.0, windowWidth,windowHeight);
    
    NSScreen *screen = [self getRiftScreen];
    mainWindow = [[MyWindow alloc] initWithContentRect:viewRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO screen:screen];
    [mainWindow setLevel:NSScreenSaverWindowLevel];
    [mainWindow setExcludedFromWindowsMenu:YES];
#ifdef ENABLE_OGRE3D
    fullScreenView = [[MyOgreView alloc] initWithFrame:viewRect];
    fullScreenView.screenshotView = screenshotView;
#else
    fullScreenView = [[MyOpenGLView alloc] initWithFrame:viewRect pixelFormat: pixelFormat];
    fullScreenView.screenshotView = screenshotView;
#endif
    
    [mainWindow setContentView: fullScreenView];
    [mainWindow makeFirstResponder:fullScreenView];
    
    initRift();

    [mainWindow setAcceptsMouseMovedEvents:YES];
    [mainWindow setMovableByWindowBackground:YES];
    [mainWindow setExcludedFromWindowsMenu:YES];
    [mainWindow makeMainWindow];
    [mainWindow makeKeyAndOrderFront:self];
    
//    double delayInSeconds = 5.0;
//    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
//    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
//        [self launchApplication:@"/Applications/TextEdit.app"];
//    });
}

- (IBAction)preferencesClicked:(id)sender {
    preferencesControllerWindow = [[PreferencesWindowController alloc] initWithWindowNibName:@"PreferencesWindow"];
//    [preferencesControllerWindow.window setLevel:NSScreenSaverWindowLevel];
    [preferencesControllerWindow showWindow:self];
}

@end
