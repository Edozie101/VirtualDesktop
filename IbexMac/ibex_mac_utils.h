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

extern "C" GLuint loadNormalTexture(const char *path_);
extern "C" GLuint loadTexture(const char *path_, bool flip=true, bool isAbsolutePath=false, bool disableAlpha=false, void *myDataIn=0, size_t widthIn=0, size_t heightIn=0);
extern "C" GLuint loadCubemapTextures(const char *path_[6]);
extern "C" GLuint createApplicationListImage(const char *path_, size_t &width, size_t &height);

#endif
