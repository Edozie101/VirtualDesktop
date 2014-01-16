//
//  PreferencesWindow.h
//  IbexMac
//
//  Created by Hesham Wahba on 1/15/14.
//  Copyright (c) 2014 Hesham Wahba. All rights reserved.
//

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#endif

#import <Foundation/Foundation.h>

@interface PreferencesWindowController : NSWindowController<NSWindowDelegate,NSTableViewDataSource,NSTableViewDelegate> {
    NSUserDefaults *defaults;
}

- (IBAction)resetButtonClicked:(id)sender;
- (IBAction)saveButtonClicked:(id)sender;

@property (nonatomic) int resolutionX;
@property (nonatomic) int resolutionY;
@property (nonatomic,retain) NSMutableArray *appLauncherFileList;

@property (nonatomic,retain) IBOutlet NSTextField *resolutionXTextField;
@property (nonatomic,retain) IBOutlet NSTextField *resolutionYTextField;

@property (nonatomic,retain) IBOutlet NSTableView *fileListTableView;

@end
