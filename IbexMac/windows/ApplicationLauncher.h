//
//  ApplicationLauncher.h
//  IbexMac
//
//  Created by Hesham Wahba on 1/10/14.
//  Copyright (c) 2014 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__ApplicationLauncher__
#define __IbexMac__ApplicationLauncher__

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "../opengl_helpers.h"
#include "../GLSLShaderProgram.h"

namespace Ibex {
    
class ApplicationLauncher
{
public:
    ApplicationLauncher();
    void update();
    void render(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);

#ifdef __APPLE__
    int processKey(unsigned short keyCode, int down);
#else
#ifdef WIN32
    int processKey(int key, int down);
#else
    int processKey(XIDeviceEvent *event, bool down);
#endif
#endif
    
private:
    size_t ww;
    size_t hh;
    GLuint appTexture;
    GLuint appSelectionTexture;
    bool first;
    
    int selectedX;
    int selectedY;
    int newX;
    int newY;
};
    
}

#endif /* defined(__IbexMac__ApplicationLauncher__) */
