//
//  ApplicationLauncher.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 1/10/14.
//  Copyright (c) 2014 Hesham Wahba. All rights reserved.
//

#include "ApplicationLauncher.h"

#ifdef WIN32
#include "../ibex_win_utils.h"
#endif
#ifdef __APPLE__
#include "../ibex_mac_utils.h"
#endif

#include "../simpleworld_plugin/SimpleWorldRendererPlugin.h"

#include <iostream>

Ibex::ApplicationLauncher::ApplicationLauncher() : ww(0),
                                                   hh(0),
                                                   appTexture(0),
                                                   appSelectionTexture(0),
                                                   selectedX(0),
                                                   selectedY(0),
                                                   first(true) {
}

void Ibex::ApplicationLauncher::update(int &selectedX_, int &selectedY_) {
    if(appSelectionTexture == 0) {
        appSelectionTexture = loadTexture("/resources/app-launcher-selection-frame.png");
    }
    if(appTexture == 0 || selectedX_ != selectedX || selectedY_ != selectedY) {
        if(appTexture) {
            glDeleteTextures(1, &appTexture);
            appTexture = 0;
        }
    #ifdef __APPLE__
        appTexture = createApplicationListImage("/Applications", ww, hh, selectedX_, selectedY_);
    #endif
        selectedX = selectedX_;
        selectedY = selectedY_;
    }
}
void Ibex::ApplicationLauncher::render(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP)
{
#ifndef __APPLE__
    return;
#endif
    
    static GLuint vaoIbexDisplayFlat = 0;
    static const GLfloat IbexDisplayFlatScale = 10;
    
    static GLint IbexDisplayFlatUniformLocations[7] = { 0, 0, 0, 0, 0, 0, 0};
    static GLint IbexDisplayFlatAttribLocations[3] = { 0, 0, 0 };
    
    static GLfloat IbexDisplayFlatVertices[] = {
        -1.0,  -1, 0.0, 0, 0, -1, 0, 0,
        1.0, -1.0, 0.0, 0, 0, -1, 1, 0,
        1.0, 1.0, 0.0, 0, 0, -1, 1, 1,
        -1.0, 1.0, 0.0, 0, 0, -1, 0, 1,
    };
    static GLuint vboIbexDisplayFlatVertices = 0;
    
    static GLushort IbexDisplayFlatIndices[] = {
        0, 1, 2,
        0, 2, 3
    };
    static GLuint vboIbexDisplayFlatIndices = 0;
    
    if(first) {
        first = false;
        
        for(int i = 0; i < sizeof(IbexDisplayFlatVertices)/sizeof(GLfloat); ++i) {
            if(i%8 < 3)
                IbexDisplayFlatVertices[i] *= IbexDisplayFlatScale;
            if(i%8 == 1)
                IbexDisplayFlatVertices[i] *= (ww != 0) ? float(hh)/float(ww) : 1.0;
        }
        
		if(standardShaderProgram.shader.program == 0) standardShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/emissive.v.glsl", "/resources/shaders/emissive.f.glsl");
        glUseProgram(standardShaderProgram.shader.program);
        
        
        IbexDisplayFlatUniformLocations[0] = glGetUniformLocation(standardShaderProgram.shader.program, "MVP");
        IbexDisplayFlatUniformLocations[1] = glGetUniformLocation(standardShaderProgram.shader.program, "V");
        IbexDisplayFlatUniformLocations[2] = glGetUniformLocation(standardShaderProgram.shader.program, "M");
        IbexDisplayFlatUniformLocations[3] = glGetUniformLocation(standardShaderProgram.shader.program, "textureIn");
        IbexDisplayFlatUniformLocations[4] = glGetUniformLocation(standardShaderProgram.shader.program, "MV");
        IbexDisplayFlatUniformLocations[5] = glGetUniformLocation(standardShaderProgram.shader.program, "inFade");
        IbexDisplayFlatUniformLocations[6] = glGetUniformLocation(standardShaderProgram.shader.program, "offset");
        
        IbexDisplayFlatAttribLocations[0] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexPosition_modelspace");
        IbexDisplayFlatAttribLocations[1] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexNormal_modelspace");
        IbexDisplayFlatAttribLocations[2] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexUV");
        
        glUseProgram(0);
        
        std::cerr << "setup_buffers" << std::endl;
        checkForErrors();
        if(vaoIbexDisplayFlat == 0) glGenVertexArrays(1,&vaoIbexDisplayFlat);
        
        checkForErrors();
        std::cerr << "gen vaoIbexDisplayFlat done" << std::endl;
        
        glBindVertexArray(vaoIbexDisplayFlat);
        if(vboIbexDisplayFlatVertices == 0) glGenBuffers(1, &vboIbexDisplayFlatVertices);
        glBindBuffer(GL_ARRAY_BUFFER, vboIbexDisplayFlatVertices);
        glBufferData(GL_ARRAY_BUFFER, sizeof(IbexDisplayFlatVertices), IbexDisplayFlatVertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[0]);
        glVertexAttribPointer(IbexDisplayFlatAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, 0);
        glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[2]);
        glVertexAttribPointer(IbexDisplayFlatAttribLocations[2], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 6));
        
        
        if(vboIbexDisplayFlatIndices == 0) glGenBuffers(1, &vboIbexDisplayFlatIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIbexDisplayFlatIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IbexDisplayFlatIndices), IbexDisplayFlatIndices, GL_STATIC_DRAW);
    }
    
    if(shadowPass) {
        glUseProgram(shadowProgram.shader.program);
        glUniformMatrix4fv(ShadowUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    } else {
        glUseProgram(standardShaderProgram.shader.program);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[1], 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[2], 1, GL_FALSE, &M[0][0]);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[4], 1, GL_FALSE, &(V*M)[0][0]);
        
        if(IbexDisplayFlatUniformLocations[5] >= 0) glUniform1f(IbexDisplayFlatUniformLocations[5], 1.0);
        if(IbexDisplayFlatUniformLocations[6] >= 0) {
            glUniform2f(IbexDisplayFlatUniformLocations[6], 0,0);
        }
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, appTexture);
        glUniform1i(IbexDisplayFlatUniformLocations[3], 0);
    }
    
    glBindVertexArray(vaoIbexDisplayFlat);
    glDrawElements(GL_TRIANGLES, sizeof(IbexDisplayFlatIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
}
