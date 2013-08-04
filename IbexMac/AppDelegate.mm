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

#import "ibex.h"

#import "ServerController.h"
#import <IOKit/graphics/IOGraphicsLib.h>

#include "OVR.h"

OVR::Ptr<OVR::DeviceManager>	pManager;
OVR::Ptr<OVR::HMDDevice>		pHMD;
OVR::Ptr<OVR::SensorDevice>	pSensor;
OVR::SensorFusion		FusionResult;
OVR::HMDInfo				Info;
bool				InfoLoaded = false;
bool				riftConnected = false;

bool modifiedDesktop(false);

static int riftX = 0;
static int riftY = 0;
static int riftResolutionX = 0;
static int riftResolutionY = 0;
void initRift() {
    OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));
    
	pManager = *OVR::DeviceManager::Create();
    
	pHMD = *pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();
    
	if (pHMD)
    {
        InfoLoaded = pHMD->GetDeviceInfo(&Info);
        
//		strncpy(Info.DisplayDeviceName, RiftMonitorName, 32);
        
		EyeDistance = Info.InterpupillaryDistance;
		DistortionK[0] = Info.DistortionK[0];
		DistortionK[1] = Info.DistortionK[1];
		DistortionK[2] = Info.DistortionK[2];
		DistortionK[3] = Info.DistortionK[3];
        
		pSensor = *pHMD->GetSensor();
        
        // This will initialize HMDInfo with information about configured IPD,
        // screen size and other variables needed for correct projection.
        // We pass HMD DisplayDeviceName into the renderer to select the
        // correct monitor in full-screen mode.
        if(InfoLoaded)
        {
            //RenderParams.MonitorName = hmd.DisplayDeviceName;
//            SConfig.SetHMDInfo(Info);
        }
	}
	else
	{
		pSensor = *pManager->EnumerateDevices<OVR::SensorDevice>().CreateDevice();
	}
    
	if (pSensor)
	{
        FusionResult.AttachToSensor(pSensor);
	FusionResult.SetPredictionEnabled(true);
	float motionPred = FusionResult.GetPredictionDelta(); // adjust in 0.01 increments
        if(motionPred < 0) motionPred = 0;
        FusionResult.SetPrediction(motionPred);
        
        if(InfoLoaded) {
            riftConnected = true;
            
            riftX = Info.DesktopX;
            riftY = Info.DesktopY;
            
            riftResolutionX = Info.HResolution;
            riftResolutionY = Info.VResolution;
        }
	}
}
void cleanUpRift() {
	pSensor.Clear();
	pManager.Clear();
    
	OVR::System::Destroy();
}

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

NSWindow* myWindow;
NSWindow *mainWindow;
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    strcpy(mResourcePath, [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding:NSUTF8StringEncoding]);
    
    CGRect mainDisplayRect = NSScreen.mainScreen.frame;
    
    [_window setFrame:mainDisplayRect display:YES];
    ScreenshotView *screenshotView = [[ScreenshotView alloc] initWithFrame:mainDisplayRect];
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
        NSOpenGLPFADoubleBuffer, 0,
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
}

@end
