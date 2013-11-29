/*
 * RendererPlugin.h
 *
 *  Created on: Sep 25, 2012
 *      Author: Hesham Wahba
 */

#ifndef RENDERERPLUGIN_H_
#define RENDERERPLUGIN_H_

#include "ibex.h"
#include "windows/Window.h"

#if defined(__APPLE__) || defined(WIN32)
typedef unsigned long Window;
typedef unsigned long GLXContext;
#endif

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
  virtual void setDesktopTexture(unsigned int desktopTexture_) {}

  // process message pump for plugin
  virtual void processEvents() {}

  // move
    virtual void move(int forward_, int right_, bool jump_, double relativeMouseX_, double relativeMouseY_) {}

  // single step in engine/world simulation
    virtual void step(const Desktop3DLocation &loc_, double timeDiff_, const double &time_) = 0;

  // do we need to swap the GLX buffer for double-buffering at the end of a render?
  virtual bool needsSwapBuffers() { return false; }
    
public:
    ::Ibex::Window window;
};

#endif /* RENDERERPLUGIN_H_ */
