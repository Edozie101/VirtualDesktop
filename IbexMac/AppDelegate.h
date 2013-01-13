//
//  AppDelegate.h
//  IbexMac
//
//  Created by Hesham Wahba on 12/25/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class MyOpenGLView;
@class ServerController;

extern "C" {
    typedef int CGSConnectionID;
    CGError CGSSetConnectionProperty(CGSConnectionID cid, CGSConnectionID targetCID, CFStringRef key, CFTypeRef value);
    int _CGSDefaultConnection();
}

@interface AppDelegate : NSObject <NSApplicationDelegate,NSWindowDelegate> {
    MyOpenGLView *fullScreenView;
    ServerController *serverController;
}

@property (assign) IBOutlet NSWindow *window;
@property (retain,nonatomic) NSOpenGLPixelFormat* pixelFormat;

@end
