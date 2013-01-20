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

@interface ViewController : UIViewController<UITextFieldDelegate,UIGestureRecognizerDelegate> {
    bool broadcast;
    
    CMMotionManager *m;
    CFSocketRef WOLsocket;
    
    IBOutlet UITextField *address;
    IBOutlet UILabel *orientation;
    IBOutlet UIButton *startStopBroadcastButton;
    
    IBOutlet UIView *orientationView;
    IBOutlet UIView *navigationView;
    UITapGestureRecognizer *tapGesture;
}


@property (readwrite, retain) NSNetServiceBrowser *browser;
@property (readwrite, retain) NSMutableArray *services;

-(IBAction) btnConnect:(id)sender;
-(IBAction) btnSend:(id)sender;
-(IBAction) doneEditing;


@end
