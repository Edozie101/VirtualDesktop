//============================================================================
// Name        : ibex.cpp
// Author      : Hesham Wahba
// Version     : 0.2
// Copyright   : Copyright Hesham Wahba 2012
// Description : IBEX Virtual Reality 3D desktop
//============================================================================

// --- Standard library ------------------------------------------------------
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
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
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>
#include <GL/glu.h>

// --- X11 -------------------------------------------------------------------
#include "opengl_setup_x11.h"

#include "opengl_helpers.h"
#include "iphone_orientation_plugin/iphone_orientation_listener.h"
#include "distortions.h"

#define HAVE_LIBJPEG 1
#include <jpeglib.h>
#include "glm/glm.h"

#include "RendererPlugin.h"

//#define ENABLE_OGRE3D 0
//#define ENABLE_IRRLICHT 1

#ifdef ENABLE_OGRE3D
#include "ogre3d_plugin/TutorialApplication.h"
#endif

#ifdef ENABLE_IRRLICHT
#include "irrlicht_plugin/irrlicht_plugin.h"
#endif

#include "simpleworld_plugin/SimpleWorldRendererPlugin.h"

#include "ibex.h"

RendererPlugin *renderer;

// TODO: get rid of global variables
Display *dpy;
Screen *scrn;
Window root;
Window overlay;

GLMmodel *pmodel = 0;

// TODO: get rid of global variables
static bool controlDesktop  = 1;

// external variables
bool showGround             = 0;
bool barrelDistort          = 0;
bool ortho                  = 1;
bool renderToTexture        = 1;
bool USE_FBO                = 1;
bool OGRE3D                 = 0;
bool IRRLICHT               = 0;
GLuint fbos[2];
GLuint textures[2];

GLuint depthBuffer;

GLuint desktopFBO;
GLuint desktopTexture(0);



GLuint cursorTexture(0);

static int xi_opcode;
static int xfixes_event_base;

double walkForward = 0;
double strafeRight = 0;


///// FROM COMPIZ
static int errors = 0;

// ===========================================================================
// Component: Interface with X11
// TODO: split into a separate file
// ===========================================================================

// ---------------------------------------------------------------------------
// Function: errorHandler
// Design:   Belongs to X11 component
// Purpose:  Sets the error handler for X11 events
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
static int errorHandler(Display *dpy, XErrorEvent *e)
{
#define DEBUG 0
#ifdef DEBUG
  char str[128];
#endif

  errors++;

#ifdef DEBUG
  XGetErrorDatabaseText (dpy, "XlibMessage", "XError", "", str, 128);
  fprintf (stderr, "%s", str);

  XGetErrorText (dpy, e->error_code, str, 128);
  fprintf (stderr, ": %s\n  ", str);

  XGetErrorDatabaseText (dpy, "XlibMessage", "MajorCode", "%d", str, 128);
  fprintf (stderr, str, e->request_code);

  sprintf (str, "%d", e->request_code);
  XGetErrorDatabaseText (dpy, "XRequest", str, "", str, 128);
  if (strcmp (str, ""))
    fprintf (stderr, " (%s)", str);
  fprintf (stderr, "\n  ");

  XGetErrorDatabaseText (dpy, "XlibMessage", "MinorCode", "%d", str, 128);
  fprintf (stderr, str, e->minor_code);
  fprintf (stderr, "\n  ");

  XGetErrorDatabaseText (dpy, "XlibMessage", "ResourceID", "%d", str, 128);
  fprintf (stderr, str, e->resourceid);
  fprintf (stderr, "\n");

  /* abort (); */
#endif
#undef DEBUG

  return 0;
}

/* TODO: Is this function still needed?
int checkForError (Display *dpy)
{
  int e;

  XSync (dpy, false);

  e = errors;
  errors = 0;

  return e;
}
*/

// ---------------------------------------------------------------------------
// Function: allow_input_passthrough
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void allow_input_passthrough(Window w)
{
  XserverRegion region = XFixesCreateRegion(dpy, 0, 0);

  XFixesSetWindowShapeRegion(dpy, w, ShapeBounding, 0, 0, 0);
  XFixesSetWindowShapeRegion(dpy, w, ShapeInput, 0, 0, region);

  XFixesDestroyRegion(dpy, region);
}


// ---------------------------------------------------------------------------
// Function: prep_root
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void prep_root(void)
{
  dpy = XOpenDisplay(0);
  display = dpy;

  bool hasNamePixmap = false;
  int event_base, error_base;
  if (XCompositeQueryExtension(dpy, &event_base, &error_base)) {
    int major = 0, minor = 2; // The highest version we support
    XCompositeQueryVersion( dpy, &major, &minor );
    hasNamePixmap = ( major > 0 || minor >= 2 );
    std::cout << "1. has composite extension! hasNamePixmap: "
              << hasNamePixmap << std::endl << std::endl;
  }

  // XInput Extension available?
  int event, error;
  if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &event, &error)) {
    std::cerr << "X Input extension not available." << std::endl;
    exit(EXIT_FAILURE);
  }

  // Which version of XI2? We support 2.0
  int major = 2, minor = 0;
  if (XIQueryVersion(dpy, &major, &minor) == BadRequest) {
    std::cerr << "XI2 not available. Server supports "
              << major << "." << minor << std::endl;
    exit(EXIT_FAILURE);
  }

  // XFixes Extension available?
  int fixesversion(5), fixeserror;
  if (!XFixesQueryExtension(dpy, &xfixes_event_base, &fixeserror)) {
    std::cerr << "X Fixes extension not available." << std::endl;
    exit(EXIT_FAILURE);
  }

  // XFixes Version available? We support 5.0
  int fixes_major(5), fixes_minor;
  if (!XFixesQueryVersion(dpy, &fixes_major, &fixes_minor)) {
    std::cerr << "X Fixes version 5 not available. Server supports "
              << fixes_major << "." << fixes_minor << std::endl;
    exit(EXIT_FAILURE);
  }

  root = DefaultRootWindow(dpy);
  scrn = DefaultScreenOfDisplay(dpy);
  screen = XDefaultScreen(dpy);
  for (int i = 0; i < ScreenCount(dpy); ++i) {
    XSelectInput(dpy, RootWindow(dpy, i), SubstructureNotifyMask |
                                          PointerMotionMask |
                                          KeyPressMask);
    XCompositeRedirectSubwindows(dpy, RootWindow(dpy, i),
                                 CompositeRedirectAutomatic);
    XSync(dpy, false);
    XIEventMask evmask;
    unsigned char mask[2] = { 0, 0 };

    XISetMask(mask, XI_HierarchyChanged | XI_Motion | XI_RawMotion);
    evmask.deviceid = XIAllDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;

    XISelectEvents(dpy, RootWindow(dpy, i), &evmask, 1);
    XFixesSelectCursorInput(dpy, RootWindow(dpy, i),
                            XFixesDisplayCursorNotifyMask);
  }

  XIEventMask evmask;
  unsigned char mask[2] = { 0, 0 };

  XISetMask(mask, XI_HierarchyChanged | XI_Motion | XI_RawMotion);
  evmask.deviceid = XIAllDevices;
  evmask.mask_len = sizeof(mask);
  evmask.mask = mask;

  XISelectEvents(dpy, DefaultRootWindow(dpy), &evmask, 1);
  XFixesSelectCursorInput(dpy, DefaultRootWindow(dpy),
                          XFixesDisplayCursorNotifyMask);
}

// ---------------------------------------------------------------------------
// Function: prep_overlay
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void prep_overlay(void)
{
  overlay = XCompositeGetOverlayWindow(dpy, root);
  allow_input_passthrough(overlay);
}


Window stage_win;

// ---------------------------------------------------------------------------
// Function: prep_stage
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void prep_stage(Window window_)
{
  XReparentWindow(dpy, window_, overlay, 0, 0);
  XSelectInput(dpy, window_, ExposureMask |
                            PointerMotionMask |
                            KeyPressMask |
                            SubstructureNotifyMask);
  allow_input_passthrough(window_);
}


Window input;

// ---------------------------------------------------------------------------
// Function: prep_input
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void prep_input(void)
{
  XWindowAttributes attr;
  XGetWindowAttributes(dpy, root, &attr);
  input = XCreateWindow(dpy, root,
                        0, 0,  /* x, y */
                        attr.width, attr.height,
                        0, 0, /* border width, depth */
                        InputOnly, DefaultVisual (dpy, 0), 0, 0);

  XSelectInput(dpy, input, StructureNotifyMask |
                            FocusChangeMask |
                            PointerMotionMask |
                            KeyPressMask |
                            KeyReleaseMask |
                            ButtonPressMask |
                            ButtonReleaseMask |
                            PropertyChangeMask);
  XMapWindow(dpy, input);
}


// ---------------------------------------------------------------------------
// Function: prep_input2
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// TODO:     Why is it even needed? Can we directly call XSetInputFocus()?
// ---------------------------------------------------------------------------
void prep_input2 (void)
{
  XSetInputFocus (dpy, window, RevertToPointerRoot, CurrentTime);
}


// ---------------------------------------------------------------------------
// Function: WaitForNotify
// Design:   Belongs to X11 component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
static Bool WaitForNotify(Display *d, XEvent *e, char *arg)
{
  return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}


// ===========================================================================
// Component: OpenGL engine
// TODO: split into a separate file
// ===========================================================================

GLfloat top(0), bottom(1);
static GLuint texture = 0;

// ---------------------------------------------------------------------------
// Class:    WindowInfo
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
typedef struct _WindowInfo
{
  XWindowAttributes attrib;
  Window window;
  Pixmap pixmap;
  GLXPixmap glxpixmap;
  GLuint texture;
  _WindowInfo() : window(0),pixmap(0),glxpixmap(0),texture(0) {}
} WindowInfo;


std::map<Window, WindowInfo> redirectedWindows;

// ---------------------------------------------------------------------------
// Function: unbindRedirectedWindow
// Design:   Interface between X11 and OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void unbindRedirectedWindow(Display *display_, Window window)
{
  if (redirectedWindows.find(window) != redirectedWindows.end()) {
    const WindowInfo &windowInfo = redirectedWindows[window];

    if (windowInfo.texture > 0)
      glDeleteTextures(1, &windowInfo.texture);

    if (windowInfo.glxpixmap > 0) {
      glXReleaseTexImageEXT (display_, windowInfo.glxpixmap, GLX_FRONT_LEFT_EXT);
      glXDestroyGLXPixmap(display_, windowInfo.glxpixmap);
    }

    if (windowInfo.pixmap > 0)
      XFreePixmap(display_, windowInfo.pixmap);

    redirectedWindows.erase(window);
  }
}


// ---------------------------------------------------------------------------
// Function: bindRedirectedWindowToTexture
// Design:   Interface between X11 and OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void bindRedirectedWindowToTexture(Display *display_, Window window_, int screen_)
{
  static const int pixmapAttribs[] = { GLX_TEXTURE_TARGET_EXT,
                                       GLX_TEXTURE_2D_EXT,
                                       GLX_TEXTURE_FORMAT_EXT,
                                       GLX_TEXTURE_FORMAT_RGBA_EXT,
                                       None };

  WindowInfo windowInfo;
  XWindowAttributes *attrib;
  if (redirectedWindows.find(window_) != redirectedWindows.end()) {
    windowInfo = redirectedWindows[window_];
  } else {
    XGetWindowAttributes( display_, window_, &windowInfo.attrib );
    if (windowInfo.attrib.map_state != IsViewable) {
      redirectedWindows[window_] = windowInfo;
      return;
    }

    Pixmap pixmap = XCompositeNameWindowPixmap(display_, window_);
    if (!pixmap)
      return;

    GLXPixmap glxpixmap = glXCreatePixmap(display_, fbconfig, pixmap,
                                          pixmapAttribs);

    GLuint tempTexture;
    glGenTextures(1, &tempTexture);
    windowInfo.pixmap    = pixmap;
    windowInfo.glxpixmap = glxpixmap;
    windowInfo.texture   = tempTexture;
    windowInfo.window    = window_;

    redirectedWindows[window_] = windowInfo;

    glBindTexture(GL_TEXTURE_2D, windowInfo.texture);
    glXBindTexImageEXT(display_, windowInfo.glxpixmap, GLX_FRONT_LEFT_EXT, 0);
  }

  attrib = &windowInfo.attrib;

  if (attrib->map_state != IsViewable)
    return;

  const double w = (double)attrib->width / (double)width;
  const double h = (double)attrib->height / (double)height;
  const double right = w;
  const double t = -h;

  glBindTexture(GL_TEXTURE_2D, windowInfo.texture);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // draw using pixmap as texture
  const double originX = (double)attrib->x/(double)width - 0.5;
  const double originY = (double)-attrib->y/(double)height + 0.5;
  const double originZ = (renderToTexture) ? 0 : -1.21;

  if(OGRE3D) {
    std::cerr << attrib->width << ", " << attrib->height << ", " << originX << ", " << originY << ", " << right << ", " << t << " -- " << windowInfo.texture << std::endl;
  }

  glColor4f(1, 1, 1, 1);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2d(0, bottom);
  glVertex3f(originX, originY, originZ);

  glTexCoord2d(1, bottom);
  glVertex3f(originX+right, originY, originZ);

  glTexCoord2d(0, top);
  glVertex3f(originX, originY + t, originZ);

  glTexCoord2d(1, top);
  glVertex3f(originX + right, originY + t, originZ);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 0);
  checkForErrors();
}

/* From: http://stackoverflow.com/questions/5002254/
                adapt-existing-code-for-opengl-stereoscopic-rendering
 * Stereo SBS projection info

glMatrixMode(GL_PROJECTION);
glLoadIdentity();
stereo_offset = eye * near * parallax_factor / convergence_distance;
glFrustum(stereo_offset + left, stereo_offset + right, bottom, top, near, far);

glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
glTranslatef(eye * parallax_factor * convergence_distance, 0, 0);

// now use gluLookAt here as this were a normal 2D rendering

 * parallax_factor should be no larger than the ratio of
 * half_eye_distance / screen_width, so the larger the screen gets the smaller
 * the parallax_factor is.
 * A good value for parallax_factor for computer display use is 0.05,
 * for large screens (think cinema) it's something like 0.01
 */

// ---------------------------------------------------------------------------
// Function: prep_framebuffers
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void prep_framebuffers()
{
  if (!GL_ARB_framebuffer_object ||
      !glewGetExtension("GL_ARB_framebuffer_object")) {
    std::cerr << "NO FBO SUPPORT" << std::endl;
    exit(EXIT_FAILURE);
  } else {
    std::cout << "GL_ARB_framebuffer_object SUPPORT" << std::endl;
  }

  glGenFramebuffers(1, &desktopFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, desktopFBO);

  if(desktopTexture == 0) {
    glGenTextures(1, &desktopTexture);
  }
  glBindTexture(GL_TEXTURE_2D, desktopTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                        GL_TEXTURE_2D, desktopTexture, 0);
  if (!checkForErrors() ||
      glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Stage 0 - Problem generating desktop FBO" << std::endl;
    exit(EXIT_FAILURE);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenFramebuffers(2, fbos);
  glGenRenderbuffers(1, &depthBuffer);
  glGenTextures(2, textures);

  for (int i = 0; i < 2; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D, textures[i], 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    if (i == 0) {
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                            width, height);
    }
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, depthBuffer);
    if (!checkForErrors()) {
      std::cerr << "Stage 1 - Problem generating FBO " << i << std::endl;
      exit(EXIT_FAILURE);
    }

    std::cout << "Generating FBO #" << i << std::endl;
    std::cout << "FBO: " << width << "x" << height << std::endl;

    if (!checkForErrors() ||
        glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cerr << "Stage 2 - Problem generating FBO " << i << std::endl;
      exit(EXIT_FAILURE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

// ---------------------------------------------------------------------------
// Function: renderDesktopToTexture
// Design:   Interface between OpenGL and Input Hardware Control component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void renderDesktopToTexture()
{
  if (renderToTexture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, desktopFBO);
    if (!checkForErrors()) {
      exit(EXIT_FAILURE);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  long unsigned int wId;
  static std::set<Window> s;

  static Window parent, root2;
  Window *children;
  static unsigned int countChildren;

  Bool mousePositionGrabbed(false);
  Window window_returned;
  int root_x, root_y;
  int win_x, win_y;
  unsigned int mask_return;

  if (ortho && renderToTexture) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 0.5, -0.5, 0.5, -10, 10);
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_DEPTH_TEST);
  }
  glPushMatrix();

  XSync(dpy, false);
  XGrabServer(dpy);
  mousePositionGrabbed = XQueryPointer(dpy, XDefaultRootWindow(dpy),
                                       &window_returned, &window_returned,
                                       &root_x, &root_y, &win_x, &win_y,
                                       &mask_return);
  XQueryTree(dpy, XDefaultRootWindow(dpy), &parent, &root2, &children,
             &countChildren);

  int d = 0;
  for (unsigned int i = 0; i < countChildren; ++i) {
    wId = children[i];
    if (wId == window || wId == overlay)
      continue;

    if (s.find(wId) == s.end()) {
      s.insert(wId);
      XCompositeRedirectSubwindows( dpy, wId, CompositeRedirectAutomatic);
      XSelectInput(dpy, wId, StructureNotifyMask |
                             PointerMotionMask |
                             ExposureMask);
    }

    if (!renderToTexture)
      glTranslatef(0, 0, d*0.0001);

    bindRedirectedWindowToTexture(dpy, wId, screen);
    ++d;
  }

  XFree(children);
  glPopMatrix();
  XUngrabServer(dpy);

  if (mousePositionGrabbed == True) {
    glEnable(GL_BLEND);
    double originX = ((double)root_x - 12) / width - 0.5;
    double originY = 0.5 - ((double)root_y + 24.0) / height;
    double originZ = -1.21 + d * 0.0001;
    double t = 24.0 / height;
    double right = 24.0 / width;

    glBindTexture(GL_TEXTURE_2D, cursorTexture);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, 1);
    glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2d(0, 1);
      glVertex3f(originX, originY, originZ);

      glTexCoord2d(1, 1);
      glVertex3f(originX + right, originY, originZ);

      glTexCoord2d(0, 0);
      glVertex3f(originX,originY+t,originZ);

      glTexCoord2d(1, 0);
      glVertex3f(originX + right, originY + t, originZ);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  if (renderToTexture) {
    if (ortho) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
//      gluPerspective(120.0f, 0.75, 0.01f, 1000.0f);
      gluPerspective(110.0f, 0.81818181, 0.01f, 1000.0f);
      glMatrixMode(GL_MODELVIEW);
      glEnable(GL_DEPTH_TEST);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}



// ---------------------------------------------------------------------------
// Function: initedOpenGL
// Design:   Belongs (mostly) to OpenGL component
// Purpose:  Whether OpenGL has been inited for rendering to texture
// Updated:  Sep 10, 2012
// TODO:     Split into separate functions
// ---------------------------------------------------------------------------
static bool initedOpenGL = false;
bool didInitOpenGL() {
//  std::cerr << "didInitOpenGL: " << initedOpenGL << std::endl;
  return initedOpenGL;
}

// ---------------------------------------------------------------------------
// Function: renderGL
// Design:   Belongs (mostly) to OpenGL component
// Purpose:  Main rendering of the 3D desktop
// Updated:  Sep 10, 2012
// TODO:     Split into separate functions
// ---------------------------------------------------------------------------
void renderGL(Desktop3DLocation& loc, double timeDiff_)
{
  static int frame = 0, time, timebase = 0, count = 0;
  frame++;
  count++;
  count %= 360;
  time = glutGet(GLUT_ELAPSED_TIME);
  if (time - timebase > 1000) {
    fprintf(stderr, "FPS:%4.2f\n", frame * 1000.0 / (time - timebase));
    timebase = time;
    frame = 0;
  }

  renderer->step(loc, timeDiff_);

  if (!initedOpenGL) {
    initedOpenGL = true;

    if(renderer->getWindowID()) window = renderer->getWindowID();
    if (OGRE3D || IRRLICHT) {
      context = glXGetCurrentContext();
      bool r = glXMakeCurrent(dpy, window, context);
      std::cerr << "R: " << r << std::endl;

      glewExperimental = true;
      GLenum err = glewInit();
      std::cerr << "Inited GLEW: " << err << std::endl;
      if (GLEW_OK != err) {
        // GLEW failed!
        exit(1);
      }

      fbconfig = getFBConfigFromContext(dpy, context);
      std::cerr << "Got fbconfig: " << fbconfig << std::endl;
    }

    bool success = init_distortion_shader();
    if (!success) {
      std::cerr << "Failed to init distortion shader!" << std::endl;
      exit(1);
    }

    if (texture == 0) {
      std::cout << "gen texture" << std::endl;
      glGenTextures(1, &texture);
      checkForErrors();
      std::cout << "gen texture done" << std::endl;
    }

    if (USE_FBO) prep_framebuffers();

    renderer->setDesktopTexture(desktopTexture);

    getCursorTexture();
  }

  if (renderToTexture && !OGRE3D && !IRRLICHT) {
    renderDesktopToTexture();
  }

  if(renderer->needsSwapBuffers()) {
    glXSwapBuffers(display, window);
  }
}

// ---------------------------------------------------------------------------
// Function: initGL
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void initGL()
{
  glShadeModel(GL_SMOOTH);
  glClearColor(0.0f, 0.1f, 0.0f, 0.0f);
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  // We use resizeGL once to set up our initial perspective
  resizeGL(width, height);

  if (GLXEW_EXT_texture_from_pixmap) {
    std::cout << "SUPPORT GLXEW_EXT_texture_from_pixmap" << std::endl;
  } else {
    std::cerr << "Don't have support for GLXEW_EXT_texture_from_pixmap"
              << std::endl;
    exit(EXIT_FAILURE);
  }

  if(glxYInverted) {
    top = 0.0f;
    bottom = 1.0f;
  }
  else {
    top = 1.0f;
    bottom = 0.0f;
  }
  top = -top;
  bottom = -bottom;

  glFlush();
}

// ---------------------------------------------------------------------------
// Function: loadMonitorModel
// Design:   Belongs to OpenGL component
// Purpose:  Loads the model of the monitor into GLM
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void loadMonitorModel()
{
  if (!pmodel) {  // load the model
    pmodel = glmReadOBJ("lcd_monitor.obj");
    if (!pmodel) {
      std::cout << "\nUsage: objviewV2 <-s> <obj filename>\n";
      exit(EXIT_FAILURE);
    }
    glmUnitize(pmodel);
    glmVertexNormals(pmodel, 90.0, GL_TRUE);
  }
}


// ---------------------------------------------------------------------------
// Function: getCursorTexture
// Design:   Belongs (mostly) to OpenGL component
// Purpose:  Converts mouse pointer from image to texture and uploads the
//           texture into OpenGL
// Updated:  Sep 10, 2012
// TODO:     Get XFixesCursorImage elsewhere and pass into this function
// ---------------------------------------------------------------------------
void getCursorTexture()
{
  if (!cursorTexture)
    glGenTextures(1, &cursorTexture);

  XFixesCursorImage *cursor_image = XFixesGetCursorImage(dpy);

  // Annoyingly, xfixes specifies the data to be 32bit, but places it in
  // an unsigned long * which can be 64 bit. So we need to iterate over a
  // 64bit structure to put it in a 32bit structure.
  // std::cerr << cursor_image->width << "x" << cursor_image->height
  //           << std::endl;
  std::vector<GLuint> pixels(cursor_image->width * cursor_image->height + 1);

  for (int i = 0; i < cursor_image->width * cursor_image->height; ++i) {
    pixels[i] = cursor_image->pixels[i] & 0xffffffff;
    pixels[i] = (pixels[i] >> 24 & 0x000000FF) ?
                      (pixels[i] | 0xFF000000) : pixels[i];
  }

  glBindTexture(GL_TEXTURE_2D, cursorTexture);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cursor_image->width,
               cursor_image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
  glBindTexture(GL_TEXTURE_2D, 0);

  XFree(cursor_image);
}


// ===========================================================================
// Component: Input Hardware Control (keyboard, mouse, hydra, tablet, etc)
// TODO: split into a separate file
// ===========================================================================

// ---------------------------------------------------------------------------
// Function: setup_hotkey
// Design:   Belongs to Input Hardware Control component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void setup_hotkey(Display *display_)
{
  unsigned int    modifiers       = ControlMask | ShiftMask;
  int             keycode         = XKeysymToKeycode(display_, XK_Y);
  Window          grab_window     = root;
  Bool            owner_events    = False;
  int             pointer_mode    = GrabModeAsync;
  int             keyboard_mode   = GrabModeAsync;

  XGrabKey(display_, keycode, modifiers, grab_window, owner_events,
           pointer_mode, keyboard_mode);

  XSelectInput(display_, root, KeyPressMask);
}


// ---------------------------------------------------------------------------
// Function: disallow_input_passthrough
// Design:   Belongs to ?
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void disallow_input_passthrough(Window w)
{
  XRectangle r;
  r.x = r.y = 0;
  r.width = width;
  r.height = height;
  XserverRegion region = XFixesCreateRegion(dpy, &r, 1);

  XFixesSetWindowShapeRegion(dpy, w, ShapeBounding, 0, 0, region);
  XFixesSetWindowShapeRegion(dpy, w, ShapeInput, 0, 0, region);

  XFixesDestroyRegion(dpy, region);
}


// ---------------------------------------------------------------------------
// Function: toggleControl
// Design:   Belongs to Input Hardware Control component
// Purpose:  Toggles user action between positional control and desktop work
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void toggleControl()
{
  controlDesktop = !controlDesktop;
  if (controlDesktop) {
    allow_input_passthrough(overlay);
    allow_input_passthrough(window);

    XUngrabKeyboard(dpy, CurrentTime);
    XUngrabPointer(dpy, CurrentTime);
  } else {
    disallow_input_passthrough(overlay);
    disallow_input_passthrough(window);

    XIEventMask eventmask;
    eventmask.deviceid = XIAllDevices;
    eventmask.mask_len = XIMaskLen(XI_RawMotion);
    std::vector<unsigned char> vecMask(eventmask.mask_len + 1, (unsigned char)0);
    eventmask.mask = &vecMask[0];

    // Now set the mask
    XISetMask(eventmask.mask, XI_Motion);
    XISetMask(eventmask.mask, XI_RawMotion);
    XISetMask(eventmask.mask, XI_KeyPress);
    XISetMask(eventmask.mask, XI_KeyRelease);

    // Select on the window
    XISelectEvents(dpy, root, &eventmask, 1);
    XISelectEvents(dpy, window, &eventmask, 1);
    XISelectEvents(dpy, overlay, &eventmask, 1);

    XIGrabDevice(dpy, XIAllDevices, root, CurrentTime,
                 0 /*cursor*/, GrabModeAsync, 0 /*pairedMode*/,
                 False, &eventmask);
    XIGrabDevice(dpy, XIAllDevices, overlay, CurrentTime,
                 0 /*cursor*/, GrabModeAsync, 0 /*pairedMode*/,
                 False, &eventmask);
    XIGrabDevice(dpy, XIAllDevices, window, CurrentTime,
                 0 /*cursor*/, GrabModeAsync, 0 /*pairedMode*/,
                 False, &eventmask);

    XGrabKeyboard(dpy, DefaultRootWindow(dpy), False, GrabModeAsync,
                  GrabModeAsync, CurrentTime);
    XGrabPointer(dpy, DefaultRootWindow(dpy), False,
                 ButtonPressMask |
                 ButtonReleaseMask |
                 PointerMotionMask |
                 FocusChangeMask |
                 EnterWindowMask |
                 LeaveWindowMask,
                 GrabModeAsync, GrabModeAsync, DefaultRootWindow(dpy), None,
                 CurrentTime);
  }
}


// ---------------------------------------------------------------------------
// Function: processKey
// Design:   Belongs to Input Hardware Control component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void processKey(XKeyEvent ke)
{
  static KeyCode toggleKey = XKeysymToKeycode(dpy, XK_Y);

  if ((ke.state & ControlMask) &&
      (ke.state & ShiftMask) &&
       ke.keycode == toggleKey) {
    toggleControl();
    return;
  }
}


// ---------------------------------------------------------------------------
// Function: debugRawMotion
// Design:   Belongs to Input Hardware Control component
// Purpose:  Prints immediate motion information for debugging purposes.
//           Not used in the release version.
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void debugRawMotion(XIRawEvent *event)
{
  double *raw_valuator = event->raw_values;
  double *valuator = event->valuators.values;

  for (int i = 0; i < event->valuators.mask_len * 8; i++) {
    if (XIMaskIsSet(event->valuators.mask, i)) {
      std::cout << "Acceleration on valuator " << i << ": "
                << *valuator - *raw_valuator << std::endl;
      valuator++;
      raw_valuator++;
    }
  }
}


// ---------------------------------------------------------------------------
// Function: processRawMotion
// Design:   Belongs to Input Hardware Control component
// Purpose:  Modifies desktop orientation/position using mouse as a tracker
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
double relativeMouseX = 0;
double relativeMouseY = 0;
void processRawMotion(XIRawEvent *event, Desktop3DLocation& loc)
{
  double *raw_valuator = event->raw_values;
  double *valuator = event->valuators.values;
  double xRotation, yRotation;

  for (int i = 0; i < event->valuators.mask_len * 8; i++) {
    if (XIMaskIsSet(event->valuators.mask, i)) {
      switch (i) {
      case 0:
        yRotation = loc.getYRotation();
        yRotation += (double)(*valuator - *raw_valuator) /
                     (double)width * 180.0;
        loc.setYRotation(yRotation);
        relativeMouseY = (double)(*valuator - *raw_valuator);
        break;
      case 1:
        xRotation = loc.getXRotation();
        xRotation += (double)(*valuator - *raw_valuator) /
                     (double)width * 180.0;
        loc.setXRotation(xRotation);
        relativeMouseX = (double)(*valuator - *raw_valuator);
        break;
      }
      valuator++;
      raw_valuator++;
    }
  }
}


// ---------------------------------------------------------------------------
// Function: processKey
// Design:   Belongs to Input Hardware Control component
// Purpose:  Modifies desktop orientation/position based on key pressed
//           Also toggles barrel distort and ground layer
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
static bool jump = false;
void processXInput2Key(XIDeviceEvent *event, bool pressed, Desktop3DLocation& loc)
{
  static KeyCode B = XKeysymToKeycode(dpy, XK_B); // toggle barrel distort
  static KeyCode G = XKeysymToKeycode(dpy, XK_G); // toggle ground

  static KeyCode W = XKeysymToKeycode(dpy, XK_W);
  static KeyCode S = XKeysymToKeycode(dpy, XK_S);
  static KeyCode A = XKeysymToKeycode(dpy, XK_A);
  static KeyCode D = XKeysymToKeycode(dpy, XK_D);
  static KeyCode Q = XKeysymToKeycode(dpy, XK_Q);
  static KeyCode E = XKeysymToKeycode(dpy, XK_E);
  static KeyCode R = XKeysymToKeycode(dpy, XK_R);
  static KeyCode SPACE = XKeysymToKeycode(dpy, XK_space);

  if (!controlDesktop) {
    if (pressed) {
      if (event->detail == W) {
        walkForward = 1;
      } else if (event->detail == S) {
        walkForward = -1;
      } else if (event->detail == A) {
        strafeRight = -1;
      } else if (event->detail == D) {
        strafeRight = 1;
      } else if (event->detail == Q) {
        strafeRight = -1;
      } else if (event->detail == E) {
        strafeRight = 1;
      } else if (event->detail == B) {
          barrelDistort = !barrelDistort;
      } else if (event->detail == G) {
          showGround = !showGround;
      } else if (event->detail == R) {
       std::cout << "RESET POSITION!" << std::endl;
       // Reset the desktop location to all zero state
       loc.resetState();
      } else if (event->detail == SPACE) {
        jump = true;
      }
    } else {
      if (walkForward ==  1 && event->detail == W)
        walkForward = 0;
      if (walkForward == -1 && event->detail == S)
        walkForward = 0;
      if (strafeRight ==  1 && (event->detail == D || event->detail == E))
        strafeRight = 0;
      if (strafeRight == -1 && (event->detail == A || event->detail == Q))
        strafeRight = 0;
      if (jump && event->detail == SPACE)
        jump = false;
    }
  }
}


// ===========================================================================
// Function: main
// Design:   Should be split into X11, OpenGL and architectural to glue them
// Purpose:  Initializes and runs the rendering loop for 3D desktop
// Updated:  Sep 10, 2012
// TODO:     Event loop should be pulled out of main. In large software
//           projects main() is to parse input arguments, initialize the code
//           and pass the control over to the engine.
// ===========================================================================
int main(int argc, char ** argv)
{
  int c;
  while ((c = getopt(argc, argv, "oih")) != -1)
    switch (c) {
    case 'o':
      OGRE3D = true;
      break;
    case 'i':
      IRRLICHT = true;
      break;
    case 'h':
    case '?':
    default:
      if (isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
      return 1;
    }


  // Instance of the class that tracks position/orientation of desktop
  Desktop3DLocation desktop3DLocation;

  prep_root();

  XEvent event;
  Bool done = False;

  XWindowAttributes attr;
  XGetWindowAttributes( dpy, root, &attr );
  width = attr.width;
  height = attr.height;

  if(OGRE3D) {
#ifdef ENABLE_OGRE3D
    renderer = new Ogre3DRendererPlugin(dpy, screen, window, visinfo, (unsigned long)context);
    renderer->init();
#endif
  } else if(IRRLICHT) {
#ifdef ENABLE_IRRLICHT
    renderer = new IrrlichtRendererPlugin();
    renderer->init();
#endif
    dpy = display;
//    irrlicht_run_loop();
  } else {
    createWindow(dpy, root);
    XIfEvent(dpy, &event, WaitForNotify, (char*)window);

    renderer = new SimpleWorldRendererPlugin();
    renderer->init();
  }

  glutInit(&argc, argv);

  prep_overlay();
  if(renderer->getWindowID()) {
    window = renderer->getWindowID();
  }
  prep_stage(window);
//  XIfEvent(dpy, &event, WaitForNotify, (char*)overlay);

  setup_iphone_listener();
  std::cerr << "dpy: " << dpy << ", display: " << display << std::endl;

  // Set X error handler here since expected errors on some window mappings
  XSetErrorHandler(errorHandler);

  XFixesHideCursor (dpy, overlay);
  setup_hotkey(dpy);

  // loadMonitorModel();

  struct timespec ts_start;
  clock_gettime(CLOCK_MONOTONIC, &ts_start);

  XEvent peekEvent;
  int pointerX, pointerY;
  unsigned int pointerMods;
  // wait for events and eat up cpu. ;-)
  while (!done) {
    // handle the events in the queue
    while (XPending(dpy) > 0) {
      XNextEvent(dpy, &event);
      XGenericEventCookie *cookie = &event.xcookie;

      if (event.xcookie.extension == xi_opcode &&
          event.xcookie.type == GenericEvent) {
        XGetEventData(dpy, cookie);
        XIDeviceEvent *xi_event = (XIDeviceEvent*)event.xcookie.data;

        switch (xi_event->evtype) {
        case XI_KeyPress:
          if (!controlDesktop) {
            processXInput2Key(xi_event, true, desktop3DLocation);
          }
          break;
        case XI_KeyRelease:
          if (!controlDesktop) {
            processXInput2Key(xi_event, false, desktop3DLocation);
          }
          break;
        case XI_RawMotion:
          if (!controlDesktop) {
            processRawMotion((XIRawEvent*)event.xcookie.data,
                             desktop3DLocation);
          }
          break;
        }

        XFreeEventData(dpy, cookie);
        continue;
      }

      if (event.xany.type == xfixes_event_base + XFixesCursorNotify) {
        XFixesCursorNotifyEvent *notify_event;
        notify_event = (XFixesCursorNotifyEvent *)(&event);

        if (notify_event->subtype == XFixesDisplayCursorNotify) {
          getCursorTexture();
        }
        continue;
      }

      switch (event.type) {
        case Expose:
        case ConfigureNotify:
        case MapNotify:
        case DestroyNotify:
        case UnmapNotify:
          unbindRedirectedWindow(dpy, event.xclient.window);
          break;

        case ButtonPress:
        case ButtonRelease:
          pointerX = event.xbutton.x_root;
          pointerY = event.xbutton.y_root;
          pointerMods = event.xbutton.state;
          break;

        case KeyPress:
        case KeyRelease:
          pointerX = event.xkey.x_root;
          pointerY = event.xkey.y_root;
          pointerMods = event.xbutton.state;
          if (event.type == KeyRelease) {
            processKey(event.xkey);
          }
          break;

        case MotionNotify:
          while (XPending (dpy)) {
            XPeekEvent (dpy, &peekEvent);
            if (peekEvent.type != MotionNotify)
              break;
            XNextEvent (dpy, &event);
          }

          pointerX = event.xmotion.x_root;
          pointerY = event.xmotion.y_root;
          pointerMods = event.xbutton.state;
          break;

        case EnterNotify:
        case LeaveNotify:
          pointerX = event.xcrossing.x_root;
          pointerY = event.xcrossing.y_root;
          pointerMods = event.xbutton.state;
          break;
        default:
          break;
      }
    }

    renderer->processEvents();

    struct timespec ts_current;
    clock_gettime(CLOCK_MONOTONIC, &ts_current);

    if (controlDesktop) {
      walkForward = strafeRight = 0;
    }

    double timeDiff = (double)(ts_current.tv_sec - ts_start.tv_sec) + (double)(ts_current.tv_nsec - ts_start.tv_nsec) * 1.0e-09;
    desktop3DLocation.walk(walkForward, strafeRight, timeDiff);

    renderer->move(walkForward, strafeRight, jump, relativeMouseX, relativeMouseY);

    renderGL(desktop3DLocation, timeDiff);

    relativeMouseX = 0;
    relativeMouseY = 0;

    ts_start = ts_current;
  }

  destroyWindow();

  return 0;
}

