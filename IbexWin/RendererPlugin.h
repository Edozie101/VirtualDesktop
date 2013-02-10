/*
 * RendererPlugin.h
 *
 *  Created on: Sep 25, 2012
 *      Author: Hesham Wahba
 */

#ifndef RENDERERPLUGIN_H_
#define RENDERERPLUGIN_H_

// --- OpenGL ----------------------------------------------------------------
#define GLX_GLXEXT_PROTOTYPES

#ifdef __APPLE__

#include "GL/glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>

typedef unsigned long Window;
typedef unsigned long GLXContext;

#else
#ifdef _WIN32

#include "GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glext.h>
#include <GL/glut.h>

typedef unsigned long Window;
typedef unsigned long GLXContext;

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

#include "ibex.h"

class RendererPlugin {
public:
  virtual ~RendererPlugin();

  // initialization after OpenGL set up
  virtual void init();

  // what is the WindowHandle or XID of the renderer plugin, return 0 for default
  virtual Window getWindowID();

  // what is the OpenGL context for the renderer, 0 for default
  virtual GLXContext getOpenGLContext();

  // set desktop texture if we are processing separately
  virtual void setDesktopTexture(GLuint desktopTexture_) {}

  // process message pump for plugin
  virtual void processEvents() {}

  // move
  virtual void move(int forward_, int right_, bool jump_, double relativeMouseX_, double relativeMouseY_) {}

  // single step in engine/world simulation
  virtual void step(const Desktop3DLocation &loc_, double timeDiff_) {}

  // do we need to swap the GLX buffer for double-buffering at the end of a render?
  virtual bool needsSwapBuffers() { return false; }
};

#endif /* RENDERERPLUGIN_H_ */
