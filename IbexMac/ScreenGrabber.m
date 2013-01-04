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
        // Initialization code here.
        t = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(refresh) userInfo:nil repeats:YES];
        
        display = kCGDirectMainDisplay; // 1
//        CGError err = CGDisplayCaptureWithOptions(display, kCGCaptureNoFill);
    }
    
    return self;
}
- (id)init
{
    self = [super init];
    if (self) {
        t = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(refresh) userInfo:nil repeats:YES];
        
        display = kCGDirectMainDisplay; // 1
//        CGError err = CGDisplayCaptureWithOptions(display, kCGCaptureNoFill);
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        t = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(refresh) userInfo:nil repeats:YES];
        
        display = kCGDirectMainDisplay; // 1
//        CGError err = CGDisplayCaptureWithOptions(display, kCGCaptureNoFill);
    }
    return self;
}
- (void) refresh {
    [self setNeedsDisplay:YES];
}

CGContextRef ctx = NULL;
- (void)drawRect:(NSRect)dirtyRect
{
//    //    CGError err = CGCaptureAllDisplaysWithOptions(kCGCaptureNoFill);
//    
////    if (err == kCGErrorSuccess)
//    {
//        //        CGSize s = CGDisplayScreenSize (display);
//        
//        CGContextRef ctx = CGDisplayGetDrawingContext (display); // 3
//        if (ctx != NULL)
//        {
//            CFArrayRef a = CGWindowListCreate(
//                                              //CGWindowListOption option,
//                                              kCGWindowListOptionOnScreenBelowWindow,
////                                              kCGWindowListOptionAll,
//                                              (CGWindowID)_window.windowNumber
//                                              //                                                      CGWindowID relativeToWindow
//                                              );
//            
//            CGImageRef i = CGWindowListCreateImageFromArray(
//                                                            CGRectInfinite,
//                                                            a,
//                                                            kCGWindowImageDefault
//                                                            );
//            //                CGImageRef i = CGWindowListCreateImage(CGRectInfinite,
//            //                                                       kCGWindowListOptionOnScreenOnly,//kCGWindowListOptionAll,
//            //                                                       kCGNullWindowID,kCGWindowImageDefault);
//            
//            //            CGImageRef i = CGDisplayCreateImage(display);
//            CGRect r = NSScreen.mainScreen.frame;//self.bounds;
//            CGContextDrawImage(ctx, r, i);
//            CGImageRelease(i);
//        }
//    }
////    CGDisplayRelease(display);
//    return;
    //////
    if (!ctx)
        ctx = [[NSGraphicsContext currentContext] graphicsPort];
    
//    CFArrayRef a = CGWindowListCreate(
//                                      //CGWindowListOption option,
//                                      kCGWindowListOptionOnScreenOnly,
//                                      kCGNullWindowID
//                                      //                                                      CGWindowID relativeToWindow
//                                      );
    CFArrayRef a = CGWindowListCreate(
                                      //CGWindowListOption option,
                                      kCGWindowListOptionOnScreenBelowWindow,
                                      (CGWindowID)_window.windowNumber
                                      //                                                      CGWindowID relativeToWindow
                                      );
    
    CGImageRef i = CGWindowListCreateImageFromArray(
                                                    CGRectInfinite,
                                                    a,
                                                    kCGWindowImageDefault
                                                    );
    
    CFRelease(a);
    //                CGImageRef i = CGWindowListCreateImage(CGRectInfinite,
    //                                                       kCGWindowListOptionOnScreenOnly,//kCGWindowListOptionAll,
    //                                                       kCGNullWindowID,kCGWindowImageDefault);
    
    //            CGImageRef i = CGDisplayCreateImage(display);
    CGRect r = NSScreen.mainScreen.frame;//self.bounds;
    CGContextDrawImage(ctx, r, i);
//    CGContextFillRect(ctx, r);
    CGImageRelease(i);
    
//    [self setNeedsDisplay:YES];
}

@end
