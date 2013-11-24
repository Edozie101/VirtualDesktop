//
//  Terrain.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 11/22/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "Terrain.h"

#include "../opengl_helpers.h"

#include <stdio.h>

void loadTerrain(unsigned char *data, int width, int height) {
    
}

void Terrain::loadHeightmap(const char * filename, int width, int height)
{
    unsigned char *data;
    FILE * file;
    
    file = fopen( filename, "rb");
    if ( file == NULL ) return;
    
    data = (unsigned char *)malloc( width * height);
    
    fread( data, width * height, 1, file );
    fclose( file );
}
