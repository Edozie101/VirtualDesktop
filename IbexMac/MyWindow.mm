//
//  MyWindow.m
//  IbexMac
//
//  Created by Hesham Wahba on 1/2/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import "MyWindow.h"
#import "ibex.h"
#import <Carbon/Carbon.h>

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

- (void)keyDown:(NSEvent *)theEvent {
    switch(theEvent.keyCode) {
        case kVK_ANSI_W:
            walkForward = 1;
            break;
        case kVK_ANSI_S:
            walkForward = -1;
            break;
        case kVK_ANSI_A:
            strafeRight = 1;
            break;
        case kVK_ANSI_D:
            strafeRight = -1;
            break;
        case kVK_Space:
            break;
    }
}
- (void)keyUp:(NSEvent *)theEvent {
    switch(theEvent.keyCode) {
        case kVK_ANSI_W:
            walkForward = 0;
            break;
        case kVK_ANSI_S:
            walkForward = 0;
            break;
        case kVK_ANSI_A:
            strafeRight = 0;
            break;
        case kVK_ANSI_D:
            strafeRight = 0;
            break;
        case kVK_Space:
            break;
    }
}

- (void)mouseMoved:(NSEvent*)theEvent {
    double x = theEvent.deltaX;
    double y = theEvent.deltaY;
    
    relativeMouseX = x;
    relativeMouseY = y;
}

@end
