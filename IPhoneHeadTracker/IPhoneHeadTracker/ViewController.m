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

@synthesize browser;
@synthesize services;

struct sockaddr_in addr4;

static CMAttitude *refAttitude;
- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [self browseServices];
    
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
    
    [self.view addSubview:navigationView];
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











//---browse for services---
-(void) browseServices {
    services = [NSMutableArray new];
    self.browser = [NSNetServiceBrowser new];
    self.browser.delegate = self;
    [self.browser searchForServicesOfType:@"_ibex._udp." inDomain:@""];
}

//---services found---
-(void)netServiceBrowser:(NSNetServiceBrowser *)aBrowser didFindService:(NSNetService *)aService moreComing:(BOOL)more {
    NSLog(@"Found service: %@", aService);
    [services addObject:aService];
    [self resolveIPAddress:aService];
}

//---services removed from the network---
-(void)netServiceBrowser:(NSNetServiceBrowser *)aBrowser didRemoveService:(NSNetService *)aService moreComing:(BOOL)more {
    [services removeObject:aService];
    [aService hostName];
}

//---resolve the IP address of a service---
-(void) resolveIPAddress:(NSNetService *)service {
    NSNetService *remoteService = service;
    remoteService.delegate = self;
    [remoteService resolveWithTimeout:0];
}

//---managed to resolve---
-(void)netServiceDidResolveAddress:(NSNetService *)service {
    NSString *name = nil;
    NSData *address_ = nil;
    struct sockaddr_in *socketAddress = nil;
    NSString *ipString = nil;
    int port;
    
    for(int i=0;i < [[service addresses] count]; i++ )
    {
        name = [service name];
        address_ = [[service addresses] objectAtIndex: i];
        socketAddress = (struct sockaddr_in *) [address_ bytes];
        ipString = [NSString stringWithFormat: @"%s", inet_ntoa(socketAddress->sin_addr)];
        port = socketAddress->sin_port;
        NSLog(@"Resolved: %@-->%@:%hu\n", [service hostName], ipString, port);
        address.text = [NSString stringWithFormat:@"%@:%hu", ipString, port];
  }
}

//---did not managed to resolve---
-(void)netService:(NSNetService *)service didNotResolve:(NSDictionary *)errorDict {
}












// called when a gesture recognizer attempts to transition out of UIGestureRecognizerStatePossible. returning NO causes it to transition to UIGestureRecognizerStateFailed
- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer {
    return YES;
}

// called when the recognition of one of gestureRecognizer or otherGestureRecognizer would be blocked by the other
// return YES to allow both to recognize simultaneously. the default implementation returns NO (by default no two gestures can be recognized simultaneously)
//
// note: returning YES is guaranteed to allow simultaneous recognition. returning NO is not guaranteed to prevent simultaneous recognition, as the other gesture's delegate may return YES
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer {
    return YES;
}

// called before touchesBegan:withEvent: is called on the gesture recognizer for a new touch. return NO to prevent the gesture recognizer from seeing this touch
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)touch {
    return YES;
}

- (IBAction)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *t = (UITouch*)[touches anyObject];
    CGPoint p0 = [t previousLocationInView:navigationView];
    CGPoint p1 = [t locationInView:navigationView];
    
    NSLog(@"Touched (%f,%f)", p1.x-p0.x, p1.y-p0.y);
}

- (IBAction)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *t = (UITouch*)[touches anyObject];
    CGPoint p0 = [t previousLocationInView:navigationView];
    CGPoint p1 = [t locationInView:navigationView];
    
    NSLog(@"Moved (%f,%f)", p1.x-p0.x, p1.y-p0.y);
}

- (IBAction)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *t = (UITouch*)[touches anyObject];
    CGPoint p0 = [t previousLocationInView:navigationView];
    CGPoint p1 = [t locationInView:navigationView];
    
    NSLog(@"Ended (%f,%f)", p1.x-p0.x, p1.y-p0.y);
}

@end
