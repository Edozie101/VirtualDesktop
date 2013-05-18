//
//  sixense_controller.h
//  IbexMac
//
//  Created by Hesham Wahba on 3/14/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//



#ifndef SIXENSE_CONTROLLER__
#define SIXENSE_CONTROLLER__

extern double sixenseStrafeRight;
extern double sixenseWalkForward;

#if _USE_SIXENSE

void mySixenseRefresh();
void myInitSixense();

#endif

#endif
