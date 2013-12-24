//
//  GLSLShaderProgram.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 11/22/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "GLSLShaderProgram.h"

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <string>

#include "utils.h"

static void show_info_log(
                          GLuint object,
                          PFNGLGETSHADERIVPROC glGet__iv,
                          PFNGLGETSHADERINFOLOGPROC glGet__InfoLog
                          )
{
    GLint log_length;
    char *log;
    
    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = (char*)malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s", log);
    free(log);
}

static GLuint make_shader(GLenum type, const char *filename)
{
    GLint length;
    GLchar *source = (GLchar*)file_contents(filename, &length);
    GLuint shader;
    GLint shader_ok;
    
    if (!source)
        return 0;
    
    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &length);
    free(source);
    glCompileShader(shader);
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok) {
        fprintf(stderr, "Failed to compile %s:\n", filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
    checkForErrors();
    std::cerr << "make_program" << std::endl;
    GLint program_ok;
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    checkForErrors();
    glValidateProgram(program);
    checkForErrors();
    
    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    checkForErrors();
    if (!program_ok) {
        fprintf(stderr, "Failed to link shader program:\n");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    } else {
        std::cerr << "Linked program" << std::endl;
    }
    
    checkForErrors();
    std::cerr << "make_program end" << std::endl;
    
    return program;
}

int GLSLShaderProgram::loadShaderProgram(const char *mResourcePath, const char *vertexShaderName, const char *fragmentShaderName)
{
    char path[2048];
    strcpy(path, mResourcePath);
#ifdef WIN32
	std::string tmp(vertexShaderName);
	std::replace(tmp.begin(), tmp.end(), '/', '\\');
	strcat(path, tmp.c_str());
#else
    strcat(path, vertexShaderName);
#endif
    std::cerr << "loading vertex shader: " << path << std::endl;
    shader.vertex_shader = make_shader(GL_VERTEX_SHADER, path);
    if (shader.vertex_shader == 0)
        return 0;
    
    strcpy(path, mResourcePath);
#ifdef WIN32
	tmp = std::string(fragmentShaderName);
	std::replace(tmp.begin(), tmp.end(), '/', '\\');
	strcat(path, tmp.c_str());
#else
    strcat(path, fragmentShaderName);
#endif
    std::cerr << "loading fragment shader: " << path << std::endl;
    shader.fragment_shader = make_shader(GL_FRAGMENT_SHADER, path);
    if (shader.fragment_shader == 0)
        return 0;
    
    shader.program = make_program(shader.vertex_shader, shader.fragment_shader);
    if (shader.program == 0)
        return 0;
    
    std::cerr << "loading shader program: success" << std::endl;
    
    return 1;
}
