//
//  Terrain.h
//  IbexMac
//
//  Created by Hesham Wahba on 11/22/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__Terrain__
#define __IbexMac__Terrain__

#include "../opengl_helpers.h"
#include "../GLSLShaderProgram.h"

#include <iostream>
#include <stdio.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Terrain {
public:
    Terrain();
    
    float getNoiseHeight(const float &x_, const float &y_) const;
    void generateNoiseTerrain(int width, int height,
                                       float scaleX_, float scaleY_, float scaleZ_,
                                       float minHeight, float maxHeight,
                                       float f1, float f2, float f3);
    
    template <class T> void loadTerrain(T *data, int width, int height);
    void loadHeightmap(const char * filename, int width=-1, int height=-1);
    
    void renderGround(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);
    
private:
    GLSLShaderProgram groundShaderProgram;
    
    unsigned char *data;
    GLfloat *vertices;
    GLuint *indices;
    GLuint numIndices;
    GLuint numVertices;
    
    GLfloat scaleX,scaleY,scaleZ;
    GLfloat translateX,translateY,translateZ;
    GLfloat minHeight, maxHeight;
    GLfloat f1, f2, f3;
    
    GLuint vaoGround = 0;
    GLuint vboGroundVertices = 0;
    GLuint vboGroundIndices = 0;
    
    GLint GroundUniformLocations[7] = { 0, 0, 0, 0, 0, 0, 0};
    GLint GroundAttribLocations[3] = { 0, 0, 0 };
};

#endif /* defined(__IbexMac__Terrain__) */
