/*
 * x11.h
 *
 *  Created on: Jul 26, 2013
 *      Author: Hesham Wahba
 */

#ifndef X11_H_
#define X11_H_

// --- Standard library ------------------------------------------------------
#include <stdlib.h>
#include <ctype.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <time.h>
#include <cstdio>
#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>
#include <set>
#include <map>
#include <vector>

// --- OpenGL ----------------------------------------------------------------
#define GLX_GLXEXT_PROTOTYPES
#ifdef __APPLE__

#include "GL/glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>

#else
#ifdef _WIN32

#include "GL/glew.h"
#include "GL/wglew.h"
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glext.h>
#include <GL/glut.h>

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

#include "../ibex.h"
#include "../opengl_setup_x11.h"

extern Display *dpy;
extern Screen *scrn;
extern Window root;
extern Window overlay;

extern int xi_opcode;
extern int xfixes_event_base;

int errorHandler(Display *dpy, XErrorEvent *e);

void allow_input_passthrough(Window w);
void prep_root(void);
void prep_overlay(void);

void prep_stage(Window window_);

void prep_input(void);
void prep_input2 (void);

Bool WaitForNotify(Display *d, XEvent *e, char *arg);

void unbindRedirectedWindow(Display *display_, Window window);
void bindRedirectedWindowToTexture(Display *display_, Window window_, int screen_);

void loadMonitorModel();
void getCursorTexture();
void setup_hotkey(Display *display_);
void disallow_input_passthrough(Window w);
void toggleControl();
void processKey(XKeyEvent ke);
void debugRawMotion(XIRawEvent *event);

void processRawMotion(XIRawEvent *event, Desktop3DLocation& loc);
void processXInput2Key(XIDeviceEvent *event, bool pressed, Desktop3DLocation& loc);

void renderDesktopToTexture();

#endif /* X11_H_ */
