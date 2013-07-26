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

// --- X11 -------------------------------------------------------------------
#include "opengl_setup_x11.h"
#include "x11/x11.h"

#include "opengl_helpers.h"
#include "iphone_orientation_plugin/iphone_orientation_listener.h"
#include "distortions.h"

#define HAVE_LIBJPEG 1
#include <jpeglib.h>
#include "glm/glm.h"

#include "RendererPlugin.h"

//#define ENABLE_OGRE3D 1
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

GLfloat top, bottom;

// TODO: get rid of global variables
DisplayShape displayShape = FlatDisplay;

bool controlDesktop  = 1;
bool showDialog = false;

// external variables
bool resetPosition          = 0;
bool showGround             = 0;
bool barrelDistort          = 0;
bool ortho                  = 1;
bool renderToTexture        = 1;
bool USE_FBO                = 1;
#ifdef ENABLE_OGRE3D
bool OGRE3D                 = 1;
#else
bool OGRE3D                 = 0;
#endif
bool IRRLICHT               = 0;
bool SBS                    = 1;

GLuint fbos[2];
GLuint textures[2];

GLuint depthBuffer;

GLuint desktopFBO;
GLuint desktopTexture(0);
GLuint videoTexture[2] = {0,0};
#ifdef _WIN32
bool mouseBlendAlternate(false);
#else
bool mouseBlendAlternate(false);
#endif
GLuint cursor(0);
int cursorSize = 32;

GLfloat cursorPosX(0);
GLfloat cursorPosY(0);

GLuint texture(0);
GLuint cursorTexture(0);

double walkForward = 0;
double strafeRight = 0;

bool done = 0;
GLfloat physicalWidth = 1440.0;
GLfloat physicalHeight = 900.0;
//GLfloat physicalWidth = 2560.0;
//GLfloat physicalHeight = 1600.0;

GLfloat videoWidth = 1;
GLfloat videoHeight = 1;
GLfloat width = 1440.0;
GLfloat height = 900.0;
GLfloat textureWidth = 1280*1.4;//1440.0*2;
GLfloat textureHeight = 800*1.4;//900.0*2;
GLfloat windowWidth = 1440;//1280;
GLfloat windowHeight = 900;//800;

double IOD = 0.1715; // at scale 0.8 // 0.136; at scale 1.0
//double IOD = 0.136; // at scale 1.0

char fpsString[32] = "FPS: -";
char mResourcePath[1024];

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

//    if (!checkForErrors()) {
//        std::cerr << "Stage 0w - Problem generating desktop FBO" << std::endl;
//        exit(EXIT_FAILURE);
//    }
    checkForErrors();
    
  glGenFramebuffers(1, &desktopFBO);
    if (!checkForErrors()) {
        std::cerr << "Stage 0z - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  glBindFramebuffer(GL_FRAMEBUFFER, desktopFBO);
    if (!checkForErrors() ) {
        std::cerr << "Stage 0y - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  if(desktopTexture == 0) {
    glGenTextures(1, &desktopTexture);
  }
  glBindTexture(GL_TEXTURE_2D, desktopTexture);
    if (!checkForErrors()) {
        std::cerr << "Stage 0a - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (!checkForErrors()) {
        std::cerr << "Stage 0b - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    if (!checkForErrors()) {
        std::cerr << "Stage 0c - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, physicalWidth, physicalHeight, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, 0);
    if (!checkForErrors()) {
        std::cerr << "Stage 0d - Problem generating desktop FBO" << std::endl;
        exit(EXIT_FAILURE);
    }
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                        GL_TEXTURE_2D, desktopTexture, 0);
  if (!checkForErrors() ||
      glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Stage 0e - Problem generating desktop FBO" << std::endl;
    exit(EXIT_FAILURE);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenFramebuffers(2, fbos);
  glGenRenderbuffers(1, &depthBuffer);
  glGenTextures(2, textures);

    for (int i = 0; i < 1; ++i) {//2; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureWidth, textureHeight, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D, textures[i], 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    if (i == 0) {
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                            textureWidth, textureHeight);
    }
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, depthBuffer);
    if (!checkForErrors()) {
      std::cerr << "Stage 1 - Problem generating FBO " << i << std::endl;
      exit(EXIT_FAILURE);
    }

    std::cout << "Generating FBO #" << i << std::endl;
    std::cout << "FBO: " << textureWidth << "x" << textureHeight << std::endl;

    if (!checkForErrors() ||
        glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cerr << "Stage 2 - Problem generating FBO " << i << std::endl;
      exit(EXIT_FAILURE);
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
void renderGL(Desktop3DLocation& loc, double timeDiff_, RendererPlugin *renderer)
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
      if(renderer->getOpenGLContext()) context = renderer->getOpenGLContext();
      if(!IRRLICHT && !OGRE3D) {
        std::cerr << "context: " << context << ", " << glXGetCurrentContext() << std::endl;
        std::cerr << "dpy: " << dpy << ", window: " << window << std::endl;
        if(!context) {
            context = glXGetCurrentContext();
        }
        std::cerr << "context: " << context << ", " << glXGetCurrentContext() << std::endl;

        bool r = glXMakeCurrent(dpy, window, context);
        std::cerr << "R: " << r << std::endl;
      }

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

  if ((renderToTexture || OGRE3D) && !IRRLICHT) {
      saveState();
    renderDesktopToTexture();
    restoreState();
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
  glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  // We use resizeGL once to set up our initial perspective
  resizeGL(windowWidth, windowHeight);//physicalWidth,physicalHeight);//width, height);

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

double relativeMouseX = 0;
double relativeMouseY = 0;

static bool jump = false;

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
  while ((c = getopt(argc, argv, "oihm")) != -1)
    switch (c) {
    case 'o':
      OGRE3D = true;
      break;
    case 'i':
      IRRLICHT = true;
      break;
    case 'm':
      SBS = false;
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
  
  getcwd(mResourcePath, sizeof(mResourcePath));

  // Instance of the class that tracks position/orientation of desktop
  Desktop3DLocation desktop3DLocation;

  prep_root();

  XEvent event;
  Bool done = False;

  XWindowAttributes attr;
  XGetWindowAttributes( dpy, root, &attr );
  width = attr.width;
  height = attr.height;
  physicalWidth = width;
  physicalHeight = height;
  std::cerr << "Virtual width: " << width << " height: " << height << std::endl;

  if(OGRE3D) {
#ifdef ENABLE_OGRE3D
    createWindow(dpy, root);
    XIfEvent(dpy, &event, WaitForNotify, (char*)window);
    bool r = glXMakeCurrent(dpy, window, context);
    std::cerr << "* glXMakeCurrent(dpy, window, context): " << r << std::endl;
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

  std::cerr << "Physical Width x Height: " << physicalWidth << "x" << physicalHeight << std::endl;

  glutInit(&argc, argv);

  prep_overlay();
  if(renderer->getWindowID()) {
    window = renderer->getWindowID();
  }
  prep_stage(window);
//  XIfEvent(dpy, &event, WaitForNotify, (char*)overlay);


  setup_iphone_listener();
  std::cerr << "dpy: " << dpy << ", display: " << display << ", " << window << std::endl;

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

    struct timespec ts_current;
    clock_gettime(CLOCK_MONOTONIC, &ts_current);

    if (controlDesktop) {
      walkForward = strafeRight = 0;
    }

    double timeDiff = (double)(ts_current.tv_sec - ts_start.tv_sec) + (double)(ts_current.tv_nsec - ts_start.tv_nsec) * 1.0e-09;
    desktop3DLocation.walk(walkForward, strafeRight, timeDiff);

    renderer->move(walkForward, strafeRight, jump, relativeMouseX, relativeMouseY);
    renderer->processEvents();

    renderGL(desktop3DLocation, timeDiff, renderer);

    relativeMouseX = 0;
    relativeMouseY = 0;

    ts_start = ts_current;
  }

  destroyWindow();

  return 0;
}

