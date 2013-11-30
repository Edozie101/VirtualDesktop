//
//  ibex_mac_utils.h
//  IbexMac
//
//  Created by Hesham Wahba on 1/1/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef IbexMac_ibex_mac_utils_h
#define IbexMac_ibex_mac_utils_h

#include <objc/objc.h>
#include <objc/message.h>

#include "opengl_helpers.h"

extern bool doubleBuffered;
extern char mResourcePath[1024];

extern "C" GLuint loadTexture(const char *path_, bool flip=true);
extern "C" GLuint loadCubemapTextures(const char *path_[6]);

#endif
