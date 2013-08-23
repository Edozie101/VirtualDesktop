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

#include <OpenGL/gl.h>

extern bool doubleBuffered;
extern char mResourcePath[1024];

extern "C" GLuint loadTexture(const char *path_);

#endif
