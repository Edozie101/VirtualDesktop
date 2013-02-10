//
//  IbexMacUtils.m
//  IbexMac
//
//  Created by Hesham Wahba on 1/1/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//
#define _CRT_SECURE_NO_WARNINGS

#include "ibex_win_utils.h"

#ifdef __APPLE__

#include "GL/glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>

#else
#ifdef _WIN32

#include "GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glext.h>
#include <GL/glut.h>

#include "SOIL/SOIL.h"

#else

#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>
#include <GL/glu.h>

#endif
#endif

#include <string.h>

char mResourcePath[1024];

GLuint loadTexture(const char *path_) {
    GLuint myTextureName = 0;
    char path[2048];
    strcpy(path, mResourcePath);
    strcat(path, path_);

	/* load an image file directly as a new OpenGL texture */
GLuint tex_2d = SOIL_load_OGL_texture
	(
		path,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		/*SOIL_FLAG_MIPMAPS | */SOIL_FLAG_INVERT_Y /*| SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT*/ | SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_TEXTURE_REPEATS
	);
return tex_2d;
}

