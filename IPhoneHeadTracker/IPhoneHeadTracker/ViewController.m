//
//  ViewController.m
//  IPhoneHeadTracker
//
//  Created by Hesham Wahba on 9/10/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import <CoreMotion/CoreMotion.h>
#import <QuartzCore/QuartzCore.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

struct sockaddr_in addr4;

static CMAttitude *refAttitude;
- (void)viewDidLoad
{
    [super viewDidLoad];
    
    NSLog(@"MOTION STARTED");
    
    m = [[CMMotionManager alloc] init];
    [m startAccelerometerUpdates];
    [m startGyroUpdates];
    [m startMagnetometerUpdates];
    [m startDeviceMotionUpdates];
    
    double delayInSeconds = 0.001;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        [self startMotion];
    });
    broadcast = false;
}

- (IBAction)toggleBroadcast:(id)sender {
    if(!broadcast) {
        refAttitude = [m deviceMotion].attitude;
        
        startStopBroadcastButton.titleLabel.text = @"Stop Broadcasting";
        
        if(WOLsocket == NULL) {
            WOLsocket = CFSocketCreate(kCFAllocatorDefault, PF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, NULL, NULL);
        }
        if ( WOLsocket == NULL) {
            NSLog(@"CfSocketCreate Failed");
        }else{
            if( WOLsocket ) {
                NSLog(@"Socket created :)");
                
                // Set up the IPv4 listening socket; port is 0, which will cause the kernel to choose a port for us.
                //struct sockaddr_in addr4;
                memset(&addr4, 0, sizeof(addr4));
                addr4.sin_len = sizeof(addr4);
                addr4.sin_family = AF_INET;
                addr4.sin_port = htons(1982);
                //            addr4.sin_addr.s_addr = htonl(INADDR_ANY);
                inet_pton(AF_INET, [address.text cStringUsingEncoding:NSASCIIStringEncoding], &addr4.sin_addr);
                //            inet_pton(AF_INET, "10.211.55.3", &addr4.sin_addr);
                //            if (kCFSocketSuccess != CFSocketSetAddress(WOLsocket, (__bridge CFDataRef) [NSData dataWithBytes:&addr4 length:sizeof(addr4)])) {
                ////                [self stop];
                //                return;
                //            }
            }
        }
        broadcast = true;
    } else {
        broadcast = false;
        startStopBroadcastButton.titleLabel.text = @"Broadcast";
    }
}

static int i = 0;
- (void)startMotion {
    ++i;
    
    CATransform3D r;
    if(refAttitude == nil) {
        refAttitude = [m deviceMotion].attitude;
    }

    CMAttitude *a = [m deviceMotion].attitude;
    [a multiplyByInverseOfAttitude:refAttitude];
    
    r.m11 = a.rotationMatrix.m11;
    r.m12 = a.rotationMatrix.m12;
    r.m13 = a.rotationMatrix.m13;
    
    r.m21 = a.rotationMatrix.m21;
    r.m22 = a.rotationMatrix.m22;
    r.m23 = a.rotationMatrix.m23;
    
    r.m31 = a.rotationMatrix.m31;
    r.m32 = a.rotationMatrix.m32;
    r.m33 = a.rotationMatrix.m33;
    
    
//    r = CATransform3DInvert(r);
    
//    CMRotationMatrix r = a.rotationMatrix;
    if(i%10 == 0) {
        //NSLog(@"r:\n%f %f %f\n%f %f %f\n%f %f %f", r.m11, r.m12, r.m13,r.m21, r.m22, r.m23,r.m31, r.m32, r.m33);
        orientation.text = [NSString stringWithFormat:@"[%0.2f %0.2f %0.2f]\n[%0.2f %0.2f %0.2f]\n[%0.2f %0.2f %0.2f]",  r.m11, r.m12, r.m13,r.m21, r.m22, r.m23,r.m31, r.m32, r.m33,nil];
    }
   
    double delayInSeconds = 1.0/60.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        [self startMotion];
        
        if(broadcast) {
            const double ethadd2[] = {r.m11, r.m12, r.m13,r.m21, r.m22, r.m23,r.m31, r.m32, r.m33};
            
            CFDataRef Data = CFDataCreate(NULL, (const UInt8*)ethadd2, sizeof(ethadd2));
    //        NSLog(@"Size of data: %d", sizeof(ethadd2));
            CFSocketSendData(WOLsocket,(__bridge CFDataRef) [NSData dataWithBytes:&addr4 length:sizeof(addr4)], Data, 0);
        }
    });
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    
    [m stopAccelerometerUpdates];
    [m stopGyroUpdates];
    [m stopMagnetometerUpdates];
    [m stopDeviceMotionUpdates];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    [textField resignFirstResponder];
    return YES;
}

@end
