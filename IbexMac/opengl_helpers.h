/*
 * opengl_helpers.h
 *
 *  Created on: Sep 12, 2012
 *      Author: Hesham Wahba
 */

#ifndef OPENGL_HELPERS_H_
#define OPENGL_HELPERS_H_

// --- OpenGL ----------------------------------------------------------------

#ifndef GLX_GLXEXT_PROTOTYPES

#define GLX_GLXEXT_PROTOTYPES

#ifdef __APPLE__

#include "GL/glew.h"
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

#else
#ifdef _WIN32

#include "GL/glew.h"
#include "GL/wglew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#else

#include <X11/Xlib.h>
#include <X11/Xregion.h>
#include <X11/Xresource.h>
#include <X11/X.h>

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

#endif

#include <stdio.h>

bool gluInvertMatrix(const double m[16], double invOut[16]);
double distort2ShaderScaleFactor(double ax, double ay);
void saveState();
void restoreState();

// ---------------------------------------------------------------------------
// Function: checkForErrors
// Design:   Belongs to OpenGL component
// Purpose:  Prints OpenGL errors
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
inline static bool checkForErrors()
{
  static bool doCheck = true;

  if (!doCheck)
    return false;

  const char* errorString = 0;
  bool retVal = false;
  GLenum error = glGetError();
  switch(error) {
    case GL_NO_ERROR:
      retVal = true;
      break;

    case GL_INVALID_ENUM:
      errorString = "GL_INVALID_ENUM";
      break;

    case GL_INVALID_VALUE:
      errorString = "GL_INVALID_VALUE";
      break;

    case GL_INVALID_OPERATION:
      errorString = "GL_INVALID_OPERATION";
      break;

    case GL_INVALID_FRAMEBUFFER_OPERATION:
      errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
      break;

    // OpenGLES Specific Errors
#ifdef ATHENA_OPENGLES
    case GL_STACK_OVERFLOW:
      errorString = "GL_STACK_OVERFLOW";
      break;

    case GL_STACK_UNDERFLOW:
      errorString = "GL_STACK_UNDERFLOW";
      break;
#endif

    case GL_OUT_OF_MEMORY:
      errorString = "GL_OUT_OF_MEMORY";
      break;

    default:
      errorString = "UNKNOWN";
      break;
  }

  if (!retVal)
      fprintf(stderr, "OpenGL ERROR: %s -- %d\n", errorString, error);

  return retVal;
}

#endif /* OPENGL_HELPERS_H_ */
