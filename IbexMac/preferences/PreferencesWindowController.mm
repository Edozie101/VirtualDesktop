//
//  PreferencesWindow.m
//  IbexMac
//
//  Created by Hesham Wahba on 1/15/14.
//  Copyright (c) 2014 Hesham Wahba. All rights reserved.
//

#import "PreferencesWindowController.h"

@implementation PreferencesWindowController

- (id)initWithWindow:(NSWindow *)window {
    self = [super initWithWindow:window];
    if(self != nil) {
        [self loadPreferences];
    }
    return self;
}

- (void)windowDidLoad {
    [self loadPreferences];
}

- (void)loadPreferences {
    defaults = [[NSUserDefaults alloc] initWithSuiteName:@"IbexSharedConfiguration"];//[NSUserDefaults standardUserDefaults];
//    [defaults addSuiteNamed:@"IbexSharedConfiguration"];
    [defaults registerDefaults:@{@"resolutionX": @1280,
                                 @"resolutionY": @800,
                                 @"appLauncherFileList": @[@"/Applications"]}];
    _resolutionX = (int)[defaults integerForKey:@"resolutionX"];
    _resolutionY = (int)[defaults integerForKey:@"resolutionY"];
    _appLauncherFileList = [NSMutableArray arrayWithArray:[defaults arrayForKey:@"appLauncherFileList"]];
    
    _resolutionXTextField.stringValue = [NSNumber numberWithInt:_resolutionX].stringValue;
    _resolutionYTextField.stringValue = [NSNumber numberWithInt:_resolutionY].stringValue;
    
    [_fileListTableView reloadData];
}
- (IBAction)resetButtonClicked:(id)sender {
    NSArray *keys = defaults.dictionaryRepresentation.allKeys;
    for(NSString *key in keys) {
        [defaults removeObjectForKey:key];
    }
    [defaults synchronize];
    [self loadPreferences];
}
- (IBAction)saveButtonClicked:(id)sender {
    _resolutionX = (int)_resolutionXTextField.integerValue;
    _resolutionY = (int)_resolutionYTextField.integerValue;
    
    [defaults setInteger:_resolutionX forKey:@"resolutionX"];
    [defaults setInteger:_resolutionY forKey:@"resolutionY"];
    [defaults setObject:_appLauncherFileList forKey:@"appLauncherFileList"];
     bool result = [defaults synchronize];
    NSLog(@"%d", result);
    [self loadPreferences];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return _appLauncherFileList.count;
}

- (NSView *)tableView:(NSTableView *)tableView
   viewForTableColumn:(NSTableColumn *)tableColumn
                  row:(NSInteger)row {
    
    // Get an existing cell with the MyView identifier if it exists
    NSTextField *result = [tableView makeViewWithIdentifier:@"DirectoryListCell" owner:self];
    
    // There is no existing cell to reuse so create a new one
    if (result == nil) {
        
        // Create the new NSTextField with a frame of the {0,0} with the width of the table.
        // Note that the height of the frame is not really relevant, because the row height will modify the height.
        result = [[NSTextField alloc] initWithFrame:CGRectZero];
        
        // The identifier of the NSTextField instance is set to MyView.
        // This allows the cell to be reused.
        result.identifier = @"DirectoryListCell";
    }
    
    // result is now guaranteed to be valid, either as a reused cell
    // or as a new cell, so set the stringValue of the cell to the
    // nameArray value at row
    result.stringValue = _appLauncherFileList[row];
    
    // Return the result
    return result;
    
}


@end
