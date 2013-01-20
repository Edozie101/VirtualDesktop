//
//  ServerController.m
//  IbexMac
//
//  Created by Hesham Wahba on 1/12/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#import "ServerController.h"

@implementation ServerController

-(void)awakeFromNib {
    [self startService];
}

-(void)startService {
    netService = [[NSNetService alloc] initWithDomain:@"" type:@"_ibex._udp."
                                                 name:@"" port:0];//]1981];
    netService.delegate = self;
    [netService publish];
}

-(void)stopService {
    [netService stop];
    netService = nil;
}

-(void)dealloc {
    [self stopService];
}

#pragma mark Net Service Delegate Methods
-(void)netService:(NSNetService *)aNetService didNotPublish:(NSDictionary *)dict {
    NSLog(@"Failed to publish: %@", dict);
}

@end
