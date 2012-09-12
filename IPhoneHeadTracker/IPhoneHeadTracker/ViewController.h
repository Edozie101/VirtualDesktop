//
//  ViewController.h
//  IPhoneHeadTracker
//
//  Created by Hesham Wahba on 9/10/12.
//  Copyright (c) 2012 Hesham Wahba. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <CFNetwork/CFNetwork.h>

@class CMMotionManager;

@interface ViewController : UIViewController {
    bool broadcast;
    
    CMMotionManager *m;
    CFSocketRef WOLsocket;
    
    IBOutlet UITextField *address;
    IBOutlet UILabel *orientation;
    IBOutlet UIButton *startStopBroadcastButton;
}

@end
