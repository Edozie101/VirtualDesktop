//
//  MyWindow.m
//  IbexMac
//
//  Created by Hesham Wahba on 1/2/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import "MyWindow.h"

@implementation MyWindow

- (BOOL)acceptsFirstResponder {
    return YES;
}
- (BOOL)canBecomeKeyWindow {
    return YES;
}
- (BOOL)canBecomeMainWindow {
    return YES;
}

@end
