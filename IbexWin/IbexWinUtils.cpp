//
//  IbexMacUtils.m
//  IbexMac
//
//  Created by Hesham Wahba on 1/1/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//
#define _CRT_SECURE_NO_WARNINGS

#include "ibex_win_utils.h"
#include "opengl_helpers.h"

#include "SOIL/SOIL.h"


#include <string.h>

bool doubleBuffered = true;
char mResourcePath[1024];

extern "C" GLuint loadNormalTexture(const char *path_) {
	return loadTexture(path_, false, false, true);
}

GLuint loadTexture(const char *path_, bool flip, bool isAbsolutePath, bool disableAlpha) {
	GLuint myTextureName = 0;
	char path[2048];
	if(isAbsolutePath) {
		strcpy(path, path_);
	} else {
		strcpy(path, mResourcePath);
		strcat(path, path_);
	}

	/* load an image file directly as a new OpenGL texture */
	GLuint tex_2d = SOIL_load_OGL_texture
		(
		path,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS |
		/*| SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT*/ 
		((flip) ? 0 : SOIL_FLAG_INVERT_Y)
		| SOIL_FLAG_MULTIPLY_ALPHA | SOIL_FLAG_TEXTURE_REPEATS
		);
	glBindTexture(GL_TEXTURE_2D, tex_2d);
	GLfloat fLargest;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
	glBindTexture(GL_TEXTURE_2D, 0);
	return tex_2d;
}

extern "C" GLuint loadCubemapTextures(const char *path_[6]) {
	GLuint myTextureName = 0;
	char path[6][2048];
	for(int i = 0; i<6;++i) {
		strcpy(path[i], mResourcePath);
		strcat(path[i], path_[i]);
	}

	/* load an image file directly as a new OpenGL texture */
	GLuint tex_2d = SOIL_load_OGL_cubemap(
		path[0],
		path[1],
		path[2],
		path[3],
		path[4],
		path[5],
		0,//SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | /*SOIL_FLAG_INVERT_Y | */SOIL_FLAG_MULTIPLY_ALPHA /*| SOIL_FLAG_TEXTURE_REPEATS*/);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_2d);
	GLfloat fLargest;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return tex_2d;
}
