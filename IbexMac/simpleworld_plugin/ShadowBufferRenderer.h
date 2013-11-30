//
//  ShadowBufferRenderer.h
//  IbexMac
//
//  Created by Hesham Wahba on 11/25/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__ShadowBufferRenderer__
#define __IbexMac__ShadowBufferRenderer__

#include "../opengl_helpers.h"

extern GLuint fboShadowMap;
extern GLuint shadowMapDepthTextureId;

void generateShadowFBO();
void bindShadowFBO();

#endif /* defined(__IbexMac__ShadowBufferRenderer__) */
