#include "RendererPlugin.h"

// --- OpenGL ----------------------------------------------------------------
#define GLX_GLXEXT_PROTOTYPES

#ifdef __APPLE__

#ifndef MY_OPENGL_DEFS
#define MY_OPENGL_DEFS
#include "GL/glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#endif

typedef unsigned long Window;
typedef unsigned long GLXContext;

#else
#ifdef _WIN32

#include "GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glext.h>

typedef unsigned long Window;
typedef unsigned long GLXContext;

#else

#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glu.h>

#endif
#endif

RendererPlugin::~RendererPlugin() {

}

void RendererPlugin::init() {

}

Window RendererPlugin::getWindowID() {
  return 0;
}

GLXContext RendererPlugin::getOpenGLContext() {
  return 0;
}
