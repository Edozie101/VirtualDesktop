/*
 * distortions.h
 *
 *  Created on: Sep 12, 2012
 *      Author: Hesham Wahba
 */

#ifndef DISTORTIONS_H_
#define DISTORTIONS_H_

#define GLX_GLXEXT_PROTOTYPES

#ifdef __APPLE__

#include "GL/glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>

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

int init_distortion_shader();
void render_distorted_frame(const bool left, const GLuint textureId);

#endif /* DISTORTIONS_H_ */
