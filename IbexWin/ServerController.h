//
//  ServerController.h
//  IbexMac
//
//  Created by Hesham Wahba on 1/12/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <Cocoa/Cocoa.h>

@interface ServerController : NSObject {
    NSNetService *netService;
}

-(void)startService;
-(void)stopService;

@end
