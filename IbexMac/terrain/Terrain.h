//
//  Terrain.h
//  IbexMac
//
//  Created by Hesham Wahba on 11/22/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__Terrain__
#define __IbexMac__Terrain__

#include <iostream>

class Terrain {
public:
    void loadHeightmap(const char * filename, int width=-1, int height=-1);
};

#endif /* defined(__IbexMac__Terrain__) */
