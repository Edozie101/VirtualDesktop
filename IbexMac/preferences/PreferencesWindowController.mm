//
//  PreferencesWindow.m
//  IbexMac
//
//  Created by Hesham Wahba on 1/15/14.
//  Copyright (c) 2014 Hesham Wahba. All rights reserved.
//

#import "PreferencesWindowController.h"

@implementation PreferencesWindowController

- (void)windowDidLoad {
    [self loadPreferences];
}

- (void)loadPreferences {
    CFStringRef appRef = CFSTR("com.hwahba.ibex.ibex");
    CFStringRef textColorKey = CFSTR("defaultTextColor");
    CFStringRef textColor;
    
    // Read the preference.
    textColor = (CFStringRef)CFPreferencesCopyAppValue(textColorKey, appRef); // kCFPreferencesCurrentApplication
    
    Boolean validValue = false;
    CFStringRef resolutionXKey = CFSTR("resolutionX");
    CFStringRef resolutionYKey = CFSTR("resolutionY");
    _resolutionX = (int)CFPreferencesGetAppIntegerValue(resolutionXKey, appRef, &validValue);
    if(!validValue) {
        _resolutionX = 1280;
    }
    _resolutionY = (int)CFPreferencesGetAppIntegerValue(resolutionYKey, appRef, &validValue);
    if(!validValue) {
        _resolutionY = 800;
    }
    
    [_resolutionXTextField setIntegerValue:_resolutionX];
    [_resolutionYTextField setIntegerValue:_resolutionY];
    
    [_fileListTableView reloadData];
}
- (IBAction)resetButtonClicked:(id)sender {
    
}
- (IBAction)saveButtonClicked:(id)sender {
    
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return 1;//nameArray.count;
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
    result.stringValue = @"/Applications";//[self.nameArray objectAtIndex:row];
    
    // Return the result
    return result;
    
}


@end
