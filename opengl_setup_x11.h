#ifndef __OPENGL_SETUP_X11_H_
#define __OPENGL_SETUP_X11_H_

#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/XEVI.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xlib.h>
#include <X11/Xregion.h>
#include <X11/Xresource.h>
#include <X11/X.h>
#include <xcb/xcb.h>
#include <xcb/xcbext.h>
/* Xlib.h is the default header that is included and has the core functionallity */
#include <X11/Xlib.h>
/* Xatom.h includes functionallity for creating new protocol messages */
#include <X11/Xatom.h>
/* keysym.h contains keysymbols which we use to resolv what keys that are being pressed */
#include <X11/keysym.h>

/* the XF86 Video Mode extension allows us to change the displaymode of the server
 * this allows us to set the display to fullscreen and also read videomodes and
 * other information.
 */
#include <X11/extensions/xf86vmode.h>

/* printf */
#include <stdio.h>

#include <cstring>

/* the XF86 Video Mode extension allows us to change the displaymode of the server
 * this allows us to set the display to fullscreen and also read videomodes and
 * other information.
 */
#include <X11/extensions/xf86vmode.h>

// --- OpenGL ----------------------------------------------------------------
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>
#include <GL/glu.h>

#define WIDTH  (1280.0)
#define HEIGHT (800.0)
//#define HEIGHT (WIDTH*900./1440.)
#define TITLE "Ibex"

typedef struct {
    Display *dpy;
    int screen;
    Window win;
    GLXContext ctx;
    XSetWindowAttributes attr;
    Bool fs;
    Bool doubleBuffered;
    XF86VidModeModeInfo deskMode;
    int x, y;
    unsigned int width, height;
    unsigned int depth;
} GLWindow;

extern GLXFBConfig fbconfig;
extern bool glxYInverted;

/* most important variable
 * it contains information about the X server which we communicate with
 */
extern Display               * display;
extern int                     screen;
/* our window instance */
extern Window                  window;
extern GLXContext              context;
//extern unsigned int            width, height;
//extern unsigned int            physicalWidth, physicalHeight;
extern Bool                    doubleBuffered;

extern XVisualInfo *visinfo;

/* prototypes */
void createWindow();
void destroyWindow();
void initGL();

void initFBConfig(Display *dpy, Window root);
void createWindow(Display *dpy_, Window root_);
void destroyWindow();
void resizeGL(unsigned int width, unsigned int height);
GLuint LoadTextureRAW( const char * filename, int wrap );

GLXFBConfig* chooseFBConfig(Display *dpy, const GLint *attribList, GLint *nElements);
XVisualInfo* getVisualFromFBConfig(Display *dpy, GLXFBConfig fbConfig);
GLXFBConfig getFBConfigFromContext(Display *dpy, GLXContext context);
GLXFBConfig getFBConfigFromVisualID(Display *dpy, VisualID visualid);

#endif
