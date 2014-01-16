//
//  AppDelegate.h
//  IbexMac
//
//  Created by Hesham Wahba on 12/25/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class PreferencesWindowController;

@interface AppDelegate : NSObject <NSApplicationDelegate,NSWindowDelegate> {
    PreferencesWindowController *controllerWindow;
}

@end
