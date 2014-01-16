//
//  AppDelegate.m
//  IbexMac
//
//  Created by Hesham Wahba on 12/25/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import "AppDelegate.h"

#include "../preferences/PreferencesWindowController.h"

@implementation AppDelegate


- (void) awakeFromNib {
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"ApplePersistenceIgnoreState"];
    
    controllerWindow = [[PreferencesWindowController alloc] initWithWindowNibName:@"PreferencesWindow"];
//    [controllerWindow.window setLevel:NSScreenSaverWindowLevel];
    [controllerWindow showWindow:self];
}

- (IBAction)preferencesClicked:(id)sender {
}

@end
