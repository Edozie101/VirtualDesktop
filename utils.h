/*
 * utils.h
 *
 *  Created on: Sep 12, 2012
 *      Author: Hesham Wahba
 */

#ifndef UTILS_H_
#define UTILS_H_

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>
#include <GL/glu.h>

void *file_contents(const char *filename, GLint *length);

#endif /* UTILS_H_ */
