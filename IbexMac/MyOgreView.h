//
//  MyOgreView.h
//  IbexMac
//
//  Created by Hesham Wahba on 3/19/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MyOgreView : NSView {
        CVDisplayLinkRef displayLink; //display link for managing rendering thread
}

@end
