//
//  GLSLShaderProgram.h
//  IbexMac
//
//  Created by Hesham Wahba on 11/22/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__GLSLShaderProgram__
#define __IbexMac__GLSLShaderProgram__

#include "opengl_helpers.h"

typedef struct {
    GLuint vertex_shader, fragment_shader, program;
    
    struct {
        GLint fade_factor;
        GLint textures[2];
    } uniforms;
    
    struct {
        GLint position;
    } attributes;
    
    GLfloat fade_factor;
} glsl_shader;

class GLSLShaderProgram {
public:
    int loadShaderProgram(const char *mResourcePath, const char *vertexShaderName, const char *fragmentShaderName);
    
    glsl_shader shader;
};

#endif /* defined(__IbexMac__GLSLShaderProgram__) */
