//
//  ShadowBufferRenderer.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 11/25/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "ShadowBufferRenderer.h"

#include <iostream>

#include "ibex.h"

GLuint fboShadowMap;
GLuint shadowMapDepthTextureId;

void bindShadowFBO() {
    glBindFramebuffer(GL_FRAMEBUFFER, fboShadowMap);
    glClear(GL_DEPTH_BUFFER_BIT);
    
//    if (!checkForErrors()) {
//        std::cerr << "GL ISSUE -- bindShadowFBO" << std::endl;
//        //exit(EXIT_FAILURE);
//    }
}

void generateShadowFBO()
{
    // size of shadow map
    int shadowMapWidth = textureWidth * 0.5;
    int shadowMapHeight = textureHeight * 0.5;
	
    glGenTextures(1, &shadowMapDepthTextureId);
    glBindTexture(GL_TEXTURE_2D, shadowMapDepthTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenFramebuffers(1, &fboShadowMap);
    glBindFramebuffer(GL_FRAMEBUFFER, fboShadowMap);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMapDepthTextureId, 0);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Problem generating shadowMapFBO" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}				
