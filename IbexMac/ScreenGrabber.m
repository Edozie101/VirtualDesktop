//
//  ScreenGrabber.m
//  IbexMac
//
//  Created by Hesham Wahba on 12/26/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import "ScreenGrabber.h"

@implementation ScreenGrabber

static CGDirectDisplayID display;
static NSTimer *t;
- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        t = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(refresh) userInfo:nil repeats:YES];
        
        display = kCGDirectMainDisplay;
    }
    
    return self;
}
- (id)init
{
    self = [super init];
    if (self) {
        t = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(refresh) userInfo:nil repeats:YES];
        
        display = kCGDirectMainDisplay;
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        t = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(refresh) userInfo:nil repeats:YES];
        
        display = kCGDirectMainDisplay;
    }
    return self;
}

- (void) refresh {
    [self setNeedsDisplay:YES];
}

CGContextRef ctx = NULL;
- (void)drawRect:(NSRect)dirtyRect
{
    if (!ctx)
        ctx = [[NSGraphicsContext currentContext] graphicsPort];

    CFArrayRef a = CGWindowListCreate(kCGWindowListOptionOnScreenBelowWindow,
                                      (CGWindowID)_window.windowNumber
                                      );
    CGImageRef i = CGWindowListCreateImageFromArray(CGRectInfinite,
                                                    a,
                                                    kCGWindowImageDefault
                                                    );
    CFRelease(a);

    CGContextDrawImage(ctx, NSScreen.mainScreen.frame, i);
    CGImageRelease(i);
}

@end
