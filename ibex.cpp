//============================================================================
// Name        : ibix.cpp
// Author      : Hesham Wahba
// Version     :
// Copyright   : Copyright Hesham Wahba 2012
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>
#include <set>
#include <map>
#include <pthread.h>

#include <time.h>


#define GLX_GLXEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>
#include <GL/glu.h>

#include "opengl_setup_x11.h"

#include "opengl_helpers.h"
#include "iphone_orientation_plugin/iphone_orientation_listener.h"

#define HAVE_LIBJPEG 1
#include <jpeglib.h>
#include "glm/glm.h"

Display *dpy;
Screen *scrn;
Window root;
Window overlay;

GLMmodel *pmodel = NULL;

double yRotation = 0.0;
double xRotation = 0.0;
double zRotation = 0.0;
double xPosition = 0.0;
double yPosition = 0.0;
double zPosition = 0.0;

static bool controlDesktop  = 1;
static bool ortho           = 1;
static bool renderToTexture = 1;
static bool USE_FBO         = 1;

static int xi_opcode;
static int xfixes_event_base;

double walkForward = 0;
double strafeRight = 0;

double WALK_SPEED = 1.0;

GLuint cursorTexture(0);

///// FROM COMPIZ
static int errors = 0;

static int
errorHandler (Display     *dpy,
	      XErrorEvent *e)
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

int
checkForError (Display *dpy)
{
    int e;

    XSync (dpy, false);

    e = errors;
    errors = 0;

    return e;
}

////////////////////

// OpenGL Errors
static inline bool CheckForErrors()
{
    static bool checkForErrors = true;

    //
    if( !checkForErrors )
    {
        return false;
    }

    //
    const char * errorString = NULL;
    bool retVal = false;

    switch( glGetError() )
    {
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

    //
    if( !retVal )
    {
        std::cerr << "OpenGL ERROR: " << errorString << std::endl;
        //        assert( retVal );
    }

    //
    return retVal;
}


void allow_input_passthrough (Window w)
{
    XserverRegion region = XFixesCreateRegion (dpy, NULL, 0);

    XFixesSetWindowShapeRegion (dpy, w, ShapeBounding, 0, 0, 0);
    XFixesSetWindowShapeRegion (dpy, w, ShapeInput, 0, 0, region);

    XFixesDestroyRegion (dpy, region);
}

void prep_root (void)
{
    dpy = XOpenDisplay(NULL);

	bool hasNamePixmap = false;
	int event_base, error_base;
	if ( XCompositeQueryExtension( dpy, &event_base, &error_base ) )
	{
		int major = 0, minor = 2; // The highest version we support
		XCompositeQueryVersion( dpy, &major, &minor );
		hasNamePixmap = ( major > 0 || minor >= 2 );
		std::cerr << "1. has composite extension! hasNamePixmap: " << hasNamePixmap << "\n" << std::endl;
	}

	/* XInput Extension available? */
	int event, error;
	if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &event, &error)) {
	   fprintf(stderr, "X Input extension not available.\n");
	   exit(-1);
	}

	/* Which version of XI2? We support 2.0 */
	int major = 2, minor = 0;
	if (XIQueryVersion(dpy, &major, &minor) == BadRequest) {
	  fprintf(stderr, "XI2 not available. Server supports %d.%d\n", major, minor);
	  exit(-1);
	}

	/* XFixes Extension available? */
	int fixesversion(5), fixeserror;
	if (!XFixesQueryExtension(dpy, &xfixes_event_base, &fixeserror)) {
	   fprintf(stderr, "X Fixes extension not available.\n");
	   exit(-1);
	}
	/* XFixes Version available? We support 5.0*/
	int fixes_major(5), fixes_minor;
	if (!XFixesQueryVersion(dpy, &fixes_major, &fixes_minor)) {
	   fprintf(stderr, "X Fixes version 5 not available. Server supports %d.%d\n", fixes_major, fixes_minor);
	   exit(-1);
	}

    root = DefaultRootWindow(dpy);
    scrn = DefaultScreenOfDisplay(dpy);
    screen = XDefaultScreen(dpy);
    for ( int i = 0; i < ScreenCount( dpy ); ++i ) {
		XSelectInput(dpy, RootWindow(dpy, i), SubstructureNotifyMask | PointerMotionMask | KeyPressMask);
		XCompositeRedirectSubwindows( dpy, RootWindow( dpy, i ), CompositeRedirectAutomatic); // Manual);//Automatic ); // CompositeRedirectManual);
		XSync(dpy, false);
//			XShapeSelectInput (dpy, root, ShapeNotifyMask);

	    XIEventMask evmask;
	    unsigned char mask[2] = { 0, 0 };

	    XISetMask(mask, XI_HierarchyChanged | XI_Motion | XI_RawMotion);
	    evmask.deviceid = XIAllDevices;
	    evmask.mask_len = sizeof(mask);
	    evmask.mask = mask;

	    XISelectEvents(dpy, RootWindow(dpy, i), &evmask, 1);
	    XFixesSelectCursorInput(dpy, RootWindow(dpy, i), XFixesDisplayCursorNotifyMask);
	}

    XIEventMask evmask;
    unsigned char mask[2] = { 0, 0 };

    XISetMask(mask, XI_HierarchyChanged | XI_Motion | XI_RawMotion);
    evmask.deviceid = XIAllDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;

    XISelectEvents(dpy, DefaultRootWindow(dpy), &evmask, 1);
    XFixesSelectCursorInput(dpy, DefaultRootWindow(dpy), XFixesDisplayCursorNotifyMask);
}

void prep_overlay (void)
{
    overlay = XCompositeGetOverlayWindow (dpy, root);
    allow_input_passthrough (overlay);
}

Window stage_win;
void prep_stage (void)
{
    XReparentWindow (dpy, window, overlay, 0, 0);
    XSelectInput (dpy, window, ExposureMask | PointerMotionMask | KeyPressMask | SubstructureNotifyMask);
    allow_input_passthrough (window);
}

Window input;
void prep_input (void)
{
    XWindowAttributes attr;

    XGetWindowAttributes (dpy, root, &attr);
    input = XCreateWindow (dpy, root,
                           0, 0,  /* x, y */
                           attr.width, attr.height,
                           0, 0, /* border width, depth */
                           InputOnly, DefaultVisual (dpy, 0), 0, NULL);

    XSelectInput (dpy, input,
                  StructureNotifyMask /*| SubstructureNotifyMask*/ | FocusChangeMask | PointerMotionMask
                  | KeyPressMask | KeyReleaseMask | ButtonPressMask
                  | ButtonReleaseMask | PropertyChangeMask);
    XMapWindow (dpy, input);
//    XSetInputFocus (dpy, input, RevertToPointerRoot, CurrentTime);

//    attach_event_source ();
}


//Pixmap pixmap = 0;
static GLfloat top(0), bottom(1);
static GLXFBConfig fbconfig;
static GLuint texture = 0;
//double b = 0;
typedef struct _WindowInfo {
	XWindowAttributes attrib;
	Window window;
	Pixmap pixmap;
	GLXPixmap glxpixmap;
	GLuint texture;
	_WindowInfo() : window(0),pixmap(0),glxpixmap(0),texture(0) { }
} WindowInfo;

std::map<Window, WindowInfo> redirectedWindows;

void unbindRedirectedWindow(Window window) {
	if(redirectedWindows.find(window) != redirectedWindows.end()) {
		WindowInfo windowInfo = redirectedWindows[window];

		if(windowInfo.texture > 0) {
			glDeleteTextures(1, &windowInfo.texture);
		}
		if(windowInfo.glxpixmap > 0) {
			glXReleaseTexImageEXT (display, windowInfo.glxpixmap, GLX_FRONT_LEFT_EXT);
			glXDestroyGLXPixmap(display, windowInfo.glxpixmap);
		}
		if(windowInfo.pixmap > 0) {
			XFreePixmap(display, windowInfo.pixmap);
		}
		redirectedWindows.erase(window);
	}
}
void bindRedirectedWindowToTexture(Display *display, Window window, int screen) {
	XWindowAttributes *attrib;

	static const int pixmapAttribs[] = { GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
					  GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
					  None };

	WindowInfo windowInfo;
	if(redirectedWindows.find(window) != redirectedWindows.end()) {
		windowInfo = redirectedWindows[window];
	} else {
		XGetWindowAttributes( display, window, &windowInfo.attrib );
		if(windowInfo.attrib.map_state != IsViewable) { //	&& !isInputOnly) { // && attr.override_redirect == 0
			redirectedWindows[window] = windowInfo;
			return;
		}

		Pixmap pixmap = XCompositeNameWindowPixmap(display, window);
		if(!pixmap) return;

		GLXPixmap glxpixmap = glXCreatePixmap (display, fbconfig, pixmap, pixmapAttribs);

		GLuint tempTexture;
		glGenTextures(1, &tempTexture);
		windowInfo.pixmap = pixmap;
		windowInfo.glxpixmap = glxpixmap;
		windowInfo.texture = tempTexture;
		windowInfo.window = window;

		redirectedWindows[window] = windowInfo;

		glBindTexture(GL_TEXTURE_2D, windowInfo.texture);
		glXBindTexImageEXT(display, windowInfo.glxpixmap, GLX_FRONT_LEFT_EXT, NULL);
	}
	attrib = &windowInfo.attrib;

	if(attrib->map_state != IsViewable) { //	&& !isInputOnly) { // && attr.override_redirect == 0
		return;
	}

	const double w = attrib->width/(double)width;
	const double h = attrib->height/(double)height;
	const double right = w;
	const double t = -h;

	glBindTexture(GL_TEXTURE_2D, windowInfo.texture);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // draw using pixmap as texture
	const double originX = attrib->x/(double)width-0.5;
	const double originY = -attrib->y/(double)height+0.5;
	const double originZ = (renderToTexture) ? 0 : -1.21;
	glColor4f(1, 1, 1, 1);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2d(0, bottom);
		glVertex3f(originX,originY,originZ);

		glTexCoord2d(1, bottom);
		glVertex3f(originX+right,originY,originZ);

		glTexCoord2d(0, top);
		glVertex3f(originX,originY+t,originZ);

		glTexCoord2d(1, top);
		glVertex3f(originX+right,originY+t,originZ);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	CheckForErrors();
}

/*From: http://stackoverflow.com/questions/5002254/adapt-existing-code-for-opengl-stereoscopic-rendering
 * Stereo SBS projection info

glMatrixMode(GL_PROJECTION);
glLoadIdentity();
stereo_offset = eye * near * parallax_factor / convergence_distance;
glFrustum(stereo_offset + left, stereo_offset + right, bottom, top, near, far);

glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
glTranslatef(eye * parallax_factor * convergence_distance, 0, 0);

// now use gluLookAt here as this were a normal 2D rendering

 *  parallax_factor should be no larger than the ratio of half_eye_distance / screen_width,
 *  so the larger the screen gets the smaller the parallax_factor is.
 *  A good value for parallax_factor for computer display use is 0.05,
 *  for large screens (think cinema) it's something like 0.01
 */

GLuint desktopFBO;
GLuint desktopTexture;

GLuint fbos[2];
GLuint textures[2];
GLuint depthBuffer;
void prep_framebuffers() {
	if(!GL_ARB_framebuffer_object || !glewGetExtension("GL_ARB_framebuffer_object")) {
		std::cerr << "NO FBO SUPPORT" << std::endl;
		exit(0);
	} else {
		std::cerr << "GL_ARB_framebuffer_object SUPPORT" << std::endl;
	}


	////
	glGenFramebuffers(1, &desktopFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, desktopFBO);

	glGenTextures(1, &desktopTexture);
	glBindTexture(GL_TEXTURE_2D, desktopTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
	                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
							 GL_TEXTURE_2D, desktopTexture,
							 0);
	if(!CheckForErrors() || glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Stage 0 - Problem generating desktop FBO" << std::endl;
		exit(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	/////

	glGenFramebuffers(2, fbos);
	glGenRenderbuffers(1, &depthBuffer);
	glGenTextures(2, textures);

	for(int i = 0; i < 2; ++i) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);

		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
		                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								 GL_TEXTURE_2D, textures[i],
								 0);

	    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	    if(i == 0) {
	    	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	    }
	    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	    if(!CheckForErrors()) {
	    	std::cerr << "Stage 1 - Problem generating FBO " << i << std::endl;
	    	exit(0);
	    }

		std::cerr << "Generating FBO #" << i << std::endl;
		std::cerr << "FBO: " << width << "x" << height << std::endl;

		if(!CheckForErrors() || glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Stage 2 - Problem generating FBO " << i << std::endl;
			exit(0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER,0);
	}
}

GLuint _skybox[6];
void loadSkybox() {
	float sizeX = 2048;
	float sizeY = 2048;
	_skybox[0] = glmLoadTexture("humus-skybox/negz.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
	_skybox[1] = glmLoadTexture("humus-skybox/posx.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
	_skybox[2] = glmLoadTexture("humus-skybox/posz.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
	_skybox[3] = glmLoadTexture("humus-skybox/negx.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
	_skybox[4] = glmLoadTexture("humus-skybox/posy.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
	_skybox[5] = glmLoadTexture("humus-skybox/negy.jpg", GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);

	std::cerr << _skybox[5] << std::endl;
}
void renderSkybox() {
    // Store the current matrix
    glPushMatrix();
    glScaled(100, 100, 100);

    // Enable/Disable features
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    // Just in case we set all vertices to white.
    glColor4f(1,1,1,1);

    // Render the front quad
    glBindTexture(GL_TEXTURE_2D, _skybox[0]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
        glTexCoord2f(0, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
    glEnd();

    // Render the left quad
    glBindTexture(GL_TEXTURE_2D, _skybox[1]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
        glTexCoord2f(0, 1); glVertex3f(  0.5f,  0.5f,  0.5f );
    glEnd();

    // Render the back quad
    glBindTexture(GL_TEXTURE_2D, _skybox[2]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f,  0.5f );
        glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f,  0.5f );

    glEnd();

    // Render the right quad
    glBindTexture(GL_TEXTURE_2D, _skybox[3]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(1, 0); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f( -0.5f,  0.5f,  0.5f );
        glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
    glEnd();

    // Render the top quad
    glBindTexture(GL_TEXTURE_2D, _skybox[4]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
        glTexCoord2f(0, 0); glVertex3f( -0.5f,  0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f,  0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
    glEnd();

    // Render the bottom quad
    glBindTexture(GL_TEXTURE_2D, _skybox[5]);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
        glTexCoord2f(0, 1); glVertex3f( -0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 1); glVertex3f(  0.5f, -0.5f,  0.5f );
        glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
    glEnd();

    // Restore enable bits and matrix
    glPopAttrib();
    glPopMatrix();
}

void renderDesktopToTexture() {
	if(renderToTexture) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, desktopFBO);
		if(!CheckForErrors()) {
			exit(1);
		}
		glClear(GL_COLOR_BUFFER_BIT);
	}

	long unsigned int wId;
	static std::set<Window> s;

	static Window parent, root2;
	Window *children;
	static unsigned int countChildren;
	static XWindowAttributes attr;

	Bool mousePositionGrabbed(false);
	Window window_returned;
	int root_x, root_y;
	int win_x, win_y;
	unsigned int mask_return;

	if(ortho && renderToTexture) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-0.5, 0.5, -0.5, 0.5, -10, 10);
		glMatrixMode(GL_MODELVIEW);
		glDisable(GL_DEPTH_TEST);
	}
	glPushMatrix();
//	glTranslatef(0, 0, -1.21);

	XSync(dpy, false);
	XGrabServer(dpy);
	mousePositionGrabbed = XQueryPointer(display, XDefaultRootWindow(dpy), &window_returned, &window_returned, &root_x, &root_y, &win_x, &win_y, &mask_return);
	XQueryTree(dpy, XDefaultRootWindow(dpy), &parent, &root2, &children, &countChildren);

	int d = 0;
	for(unsigned int i = 0; i < countChildren; ++i) {
		wId = children[i];
		if(wId == window || wId == overlay) continue;

		if(s.find(wId) == s.end()) {
			s.insert(wId);
			XCompositeRedirectSubwindows( dpy, wId, CompositeRedirectAutomatic); // Manual);
			XSelectInput(dpy, wId, StructureNotifyMask | PointerMotionMask | ExposureMask);
		}
		XGetWindowAttributes( dpy, wId, &attr );

//		#if defined(__cplusplus) || defined(c_plusplus)
//			bool isInputOnly = attr.c_class == InputOnly;
//		#else
//			bool isInputOnly = attr.class == InputOnly;
//		#endif
		if(!renderToTexture) glTranslatef(0, 0, d*0.0001);
		bindRedirectedWindowToTexture(dpy, wId, screen);
		++d;
	}
	XFree(children);
	glPopMatrix();
	XUngrabServer(dpy);

	if(mousePositionGrabbed == True) {
		glEnable(GL_BLEND);
		double originX = ((double)root_x-12)/width-0.5;
		double originY = 0.5-((double)root_y+24.0)/height;
		double originZ = -1.21+d*0.0001;
		double t = 24.0/height;
		double right = 24.0/width;
		glBindTexture(GL_TEXTURE_2D, cursorTexture);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1, 1, 1, 1);
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2d(0, 1);
			glVertex3f(originX,originY,originZ);

			glTexCoord2d(1, 1);
			glVertex3f(originX+right,originY,originZ);

			glTexCoord2d(0, 0);
			glVertex3f(originX,originY+t,originZ);

			glTexCoord2d(1, 0);
			glVertex3f(originX+right,originY+t,originZ);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);

		glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	if(renderToTexture) {
		if(ortho) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
	//			gluPerspective(45.0f, 1, 0.1f, 1000.0f);//(GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
			gluPerspective(120.0f, 0.75, 0.01f, 1000.0f);
			glMatrixMode(GL_MODELVIEW);
			glEnable(GL_DEPTH_TEST);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void renderGL(void) {
	static int frame=0,time,timebase=0, count=0;
	frame++;
	count++;
	count %= 360;
	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
		fprintf(stderr,"FPS:%4.2f\n", frame*1000.0/(time-timebase));
		timebase = time;
		frame = 0;
	}

	static XWindowAttributes attr;

	static int value;
	static int nfbconfigs;
	static GLXFBConfig *fbconfigs = glXGetFBConfigs (display, screen, &nfbconfigs);
	static int i = 0;

	XGetWindowAttributes( dpy, root, &attr );
	static bool init = false;
	if(!init) {
		init = true;
		int attrib[] = {
		        GLX_RENDER_TYPE, GLX_RGBA_BIT,
		        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		        GLX_RED_SIZE, 1,
		        GLX_GREEN_SIZE, 1,
		        GLX_BLUE_SIZE, 1,
		        GLX_ALPHA_SIZE, 1,
		        GLX_DOUBLEBUFFER, True,
		        GLX_DEPTH_SIZE, 1,
		        None };
//		    GLXFBConfig *fbconfigs, fbconfig;
		int numfbconfigs, render_event_base, render_error_base;
		XVisualInfo *visinfo;
		XRenderPictFormat *pictFormat;

		/* Make sure we have the RENDER extension */
		if(!XRenderQueryExtension(dpy, &render_event_base, &render_error_base)) {
			fprintf(stderr, "No RENDER extension found\n");
			exit(EXIT_FAILURE);
		}


		/* Get the list of FBConfigs that match our criteria */
		int scrnum = 0;
		fbconfigs = glXChooseFBConfig(dpy, scrnum, attrib, &numfbconfigs);
		if (!fbconfigs) {
			/* None matched */
			exit(EXIT_FAILURE);
		}

		/* Find an FBConfig with a visual that has a RENDER picture format that
		 * has alpha */
		for (i = 0; i < numfbconfigs; i++) {
			visinfo = glXGetVisualFromFBConfig(dpy, fbconfigs[i]);
			if (!visinfo) continue;
			pictFormat = XRenderFindVisualFormat(dpy, visinfo->visual);
			if (!pictFormat) continue;

			if(pictFormat->direct.alphaMask > 0) {
				fbconfig = fbconfigs[i];
				break;
			}

			XFree(visinfo);
		}

		if (i == numfbconfigs) {
			/* None of the FBConfigs have alpha.  Use a normal (opaque)
			 * FBConfig instead */
			fbconfig = fbconfigs[0];
			visinfo = glXGetVisualFromFBConfig(dpy, fbconfig);
			pictFormat = XRenderFindVisualFormat(dpy, visinfo->visual);
		}

			glXGetFBConfigAttrib (dpy, fbconfigs[i],
								  GLX_Y_INVERTED_EXT,
								  &value);
			if (value == TRUE)
			{
				top = 0.0f;
				bottom = 1.0f;
			}
			else
			{
				top = 1.0f;
				bottom = 0.0f;
			}


		glEnable(GL_TEXTURE_2D);

		top = -top;
		bottom = -bottom;

		fbconfig = fbconfigs[i];

		XFree(fbconfigs);
	}

	if(renderToTexture) {
		renderDesktopToTexture();
	}

	for(int i2 = 0; i2 < 2; ++i2) {
		if(USE_FBO) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbos[i2]);
			if(!CheckForErrors()) {
				std::cerr << "GL ISSUE" << std::endl;
				exit(0);
			}
			glPushMatrix();
		} else {
			if(i2 > 0) break;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glPushMatrix();
		{
			double orientation[16];
			gluInvertMatrix(get_orientation(), orientation);
			glMultMatrixd(orientation);
			glRotated(xRotation, 1, 0, 0);
			glRotated(yRotation, 0, 1, 0);

			glPushMatrix();
			{
				renderSkybox();
				glTranslated(xPosition, yPosition, zPosition);
				glTranslated((i2 == 0) ? -0.01 : 0.01, 0, 0);

				glPushMatrix();
				{

					/* Lighting Variables */
					const GLfloat light_ambient[] = {
						0.0, 0.0, 0.0, 1.0
					};

					const GLfloat light_diffuse[] = {
						1.0, 1.0, 1.0, 1.0
					};

					const GLfloat light_specular[] = {
						1.0, 1.0, 1.0, 1.0
					};

					const GLfloat light_position[] = {
						1.0, 1.0, 1.0, 0.0
					};

					const GLfloat mat_ambient[] = {
						0.7, 0.7, 0.7, 1.0
					};

					const GLfloat mat_diffuse[] = {
						0.8, 0.8, 0.8, 1.0
					};

					const GLfloat mat_specular[] = {
						1.0, 1.0, 1.0, 1.0
					};

					const GLfloat high_shininess[] = {
						100.0
					};

//					glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
//					glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
//					glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
//					glLightfv(GL_LIGHT0, GL_POSITION, light_position);
//					glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
//					glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
//					glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
//					glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
//					glEnable(GL_LIGHTING);
//					glEnable(GL_LIGHT0);
//					glDepthFunc(GL_LESS);
//					glEnable(GL_DEPTH_TEST);
//					glEnable(GL_NORMALIZE);
//
//					glEnable(GL_LIGHTING);
//					glPushMatrix();
//					{
//						glTranslated(0, -0.1, -2.21);
//						glRotated(90, 0, 1, 0);
//						glScaled(1, 0.72, 0.6);
//						glPushMatrix();
//						{
////							glmDraw(pmodel, GLM_FLAT | GLM_COLOR);
//						}
//						glPopMatrix();
//					}
//					glPopMatrix();
//					glDisable(GL_LIGHTING);

					if(renderToTexture) {
						double ySize = ((double)height/(double)width)/2.0;
						const double monitorOriginZ = -0.5;
						glBindTexture(GL_TEXTURE_2D, desktopTexture);
						glBegin(GL_TRIANGLE_STRIP);
							glTexCoord2d(0, 0);
							glVertex3f(-0.5,-ySize,monitorOriginZ);

							glTexCoord2d(1, 0);
							glVertex3f(0.5,-ySize,monitorOriginZ);

							glTexCoord2d(0, 1);
							glVertex3f(-0.5,ySize,monitorOriginZ);

							glTexCoord2d(1, 1);
							glVertex3f(0.5,ySize,monitorOriginZ);
						glEnd();
						glBindTexture(GL_TEXTURE_2D, 0);
					} else {
						renderDesktopToTexture();
					}
				}
				glPopMatrix();
			}
			glPopMatrix();
		}
		glPopMatrix();

		if(USE_FBO) {
			glPopMatrix();
		}
	}


	if(USE_FBO) {
		if(ortho) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, 1, 0, 1, -1, 1);
			glMatrixMode(GL_MODELVIEW);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor4f(1, 1, 1, 1);
		for(int i = 0; i < 2; ++i) {
			if(ortho) {
				double originX = (i == 0) ? 0 : 0.5;
				glBindTexture(GL_TEXTURE_2D, textures[i]);
				glPushMatrix();
//				glTranslated((i < 1) ? -0.98 : 0, -0.5, -2.4);
				glColor4f(1, 1, 1, 1);
				glBegin(GL_TRIANGLE_STRIP);
					glTexCoord2d(0, bottom);
					glVertex3f(originX,0,0);

					glTexCoord2d(1, bottom);
					glVertex3f(originX+0.5,0,0);

					glTexCoord2d(0, top);
					glVertex3f(originX,1,0);

					glTexCoord2d(1, top);
					glVertex3f(originX+0.5,1,0);
				glEnd();
				glPopMatrix();
			} else {
//				glPushMatrix();
//				glTranslated((i == 0) ? -0.01 : 0.01, 0, 0);
//
//				glBindTexture(GL_TEXTURE_2D, 0);
//				glPushMatrix();
//				glTranslated((i < 1) ? -0.5 : 0.5, 0.7, -2.2);
//				glRotated(count*2%360, 0, 1, 0);
//				glutWireTeapot(0.1);
//				glPopMatrix();

				glBindTexture(GL_TEXTURE_2D, textures[i]);
				glPushMatrix();
				glTranslated((i < 1) ? -0.98 : 0, -0.5, -2.4);
				glColor4f(1, 1, 1, 1);
				glBegin(GL_TRIANGLE_STRIP);
					glTexCoord2d(0, bottom);
					glVertex3f(0,0,0);

					glTexCoord2d(1, bottom);
					glVertex3f(1,0,0);

					glTexCoord2d(0, top);
					glVertex3f(0,1,0);

					glTexCoord2d(1, top);
					glVertex3f(1,1,0);
				glEnd();
				glPopMatrix();

//				glPopMatrix();
			}
		}
		if(ortho) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
//			gluPerspective(45.0f, 1, 0.1f, 1000.0f);//(GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
		    gluPerspective(120.0f, 0.75, 0.01f, 1000.0f);
			glMatrixMode(GL_MODELVIEW);
		}
	}

	if (doubleBuffered)
	{
		glXSwapBuffers(display, window);
	}
}

void initGL()
{
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.1f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    // we use resizeGL once to set up our initial perspective
    resizeGL(width, height);

	if(GLXEW_EXT_texture_from_pixmap) {
		std::cerr << "SUPPORT GLXEW_EXT_texture_from_pixmap" << std::endl;
	} else {
		std::cerr << "Don't have support for GLXEW_EXT_texture_from_pixmap" << std::endl;
		exit(0);
	}

    glFlush();
}

void prep_input2 (void)
{
//    XSelectInput (dpy, root, PointerMotionMask);

//
//    XSelectInput (dpy, root,
//                      StructureNotifyMask | FocusChangeMask | PointerMotionMask
//                      | KeyPressMask | KeyReleaseMask //| ButtonPressMask
//                      | ButtonReleaseMask
//                      | PropertyChangeMask);
//	XMapWindow (dpy, root);
//	XSetInputFocus (dpy, root, RevertToPointerRoot, CurrentTime);
	XSetInputFocus (dpy, window, RevertToPointerRoot, CurrentTime);
}

void loadModel() {
	if (!pmodel) {		/* load up the model */
		pmodel = glmReadOBJ("lcd_monitor.obj");
		if (!pmodel) {
		    printf("\nUsage: objviewV2 <-s> <obj filename>\n");
		    exit(0);
		}
		glmUnitize(pmodel);
		glmVertexNormals(pmodel, 90.0, GL_TRUE);
	}
}
void setup_hotkey(Display *display_) {
	unsigned int    modifiers       = ControlMask | ShiftMask;
	int             keycode         = XKeysymToKeycode(display_,XK_Y);
	Window          grab_window     =  root;
	Bool            owner_events    = False;
	int             pointer_mode    = GrabModeAsync;
	int             keyboard_mode   = GrabModeAsync;

	XGrabKey(display_, keycode, modifiers, grab_window, owner_events, pointer_mode,
			 keyboard_mode);

	XSelectInput(display_, root, KeyPressMask );
}

void disallow_input_passthrough(Window w) {
	XRectangle r;
	r.x = r.y = 0;
	r.width = width;
	r.height = height;
	XserverRegion region = XFixesCreateRegion (dpy, &r, 1);


	XFixesSetWindowShapeRegion (dpy, w, ShapeBounding, 0, 0, region);
	XFixesSetWindowShapeRegion (dpy, w, ShapeInput, 0, 0, region);
//	XFixesSetWindowShapeRegion (dpy, w, None, 0, 0, region);

	XFixesDestroyRegion(dpy, region);
}

void toggleControl() {
	controlDesktop = !controlDesktop;
	if(controlDesktop) {
		allow_input_passthrough (overlay);
		allow_input_passthrough (window);

		XUngrabKeyboard(dpy, CurrentTime);
		XUngrabPointer(dpy, CurrentTime);
	} else {
		disallow_input_passthrough (overlay);
		disallow_input_passthrough (window);


		XIEventMask eventmask;
		eventmask.deviceid = XIAllDevices;
		eventmask.mask_len = XIMaskLen(XI_RawMotion);
		eventmask.mask = (unsigned char *)calloc(eventmask.mask_len, sizeof(char));
		/* now set the mask */
//		XISetMask(eventmask.mask, XI_ButtonPress);
		XISetMask(eventmask.mask, XI_Motion);
		XISetMask(eventmask.mask, XI_RawMotion);
		XISetMask(eventmask.mask, XI_KeyPress);
		XISetMask(eventmask.mask, XI_KeyRelease);

		/* select on the window */
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

		XGrabKeyboard(dpy, DefaultRootWindow(dpy),
		                 False, GrabModeAsync, GrabModeAsync, CurrentTime);
		XGrabPointer(dpy, DefaultRootWindow(dpy),
		                 False,  ButtonPressMask |
		                 ButtonReleaseMask |
		                 PointerMotionMask |
		                 FocusChangeMask |
		                 EnterWindowMask |
		                  LeaveWindowMask
		                  , GrabModeAsync, GrabModeAsync, DefaultRootWindow(dpy), None, CurrentTime);

		free(eventmask.mask);
	}
}

void walk(double forward, double right, double seconds) {
	xPosition -= sin(yRotation/90.0*M_PI_2) * WALK_SPEED * seconds * forward;
	zPosition += cos(yRotation/90.0*M_PI_2) * WALK_SPEED * seconds * forward;

	xPosition -= cos(-yRotation/90.0*M_PI_2) * WALK_SPEED * seconds * right;
	zPosition += sin(-yRotation/90.0*M_PI_2) * WALK_SPEED * seconds * right;
}

void processKey(XKeyEvent ke) {
	static KeyCode toggleKey = XKeysymToKeycode(dpy,XK_Y);

	if((ke.state & ControlMask) && (ke.state & ShiftMask) && ke.keycode == toggleKey) {
		toggleControl();
		return;
	}
}

void print_rawmotion(XIRawEvent *event)
{
    int i;
    double *raw_valuator = event->raw_values,
           *valuator = event->valuators.values;

    for (i = 0; i < event->valuators.mask_len * 8; i++) {
        if (XIMaskIsSet(event->valuators.mask, i)) {
            printf("Acceleration on valuator %d: %f\n",
                   i, *valuator - *raw_valuator);
            valuator++;
            raw_valuator++;
        }
    }
}

void process_rawmotion(XIRawEvent *event) {
	double *raw_valuator = event->raw_values;
	double *valuator = event->valuators.values;

	for (int i = 0; i < event->valuators.mask_len * 8; i++) {
		if (XIMaskIsSet(event->valuators.mask, i)) {
			switch(i) {
			case 0:
				yRotation += (double)(*valuator - *raw_valuator)/(double)width * 180.0;;
				break;
			case 1:
				xRotation += (double)(*valuator - *raw_valuator)/(double)width * 180.0;
				break;
			}
			valuator++;
			raw_valuator++;
		}
	}
}

void processKey(XIDeviceEvent *event, bool pressed) {
	static KeyCode W = XKeysymToKeycode(dpy,XK_W);
	static KeyCode S = XKeysymToKeycode(dpy,XK_S);
	static KeyCode A = XKeysymToKeycode(dpy,XK_A);
	static KeyCode D = XKeysymToKeycode(dpy,XK_D);
	static KeyCode Q = XKeysymToKeycode(dpy,XK_Q);
	static KeyCode E = XKeysymToKeycode(dpy,XK_E);

	static KeyCode R = XKeysymToKeycode(dpy,XK_R);

	if(!controlDesktop) {
		if(pressed) {
			if(event->detail == W) {
				walkForward = 1;
			} else if(event->detail == S) {
				walkForward = -1;
			} else if(event->detail == A) {
				strafeRight = -1;
			} else if(event->detail == D) {
				strafeRight = 1;
			} else if(event->detail == Q) {
				strafeRight = -1;
			} else if(event->detail == E) {
				strafeRight = 1;
			} else if(event->detail == R) {
				std::cerr << "RESET POSITION!" << std::endl;
				xRotation = 0;
				yRotation = 0;
				zRotation = 0;
				xPosition = 0;
				yPosition = 0;
				zPosition = 0;
			}
		} else {
			if(walkForward ==  1 && event->detail == W) walkForward = 0;
			if(walkForward == -1 && event->detail == S) walkForward = 0;
			if(strafeRight ==  1 && (event->detail == D || event->detail == E)) strafeRight = 0;
			if(strafeRight == -1 && (event->detail == A || event->detail == Q)) strafeRight = 0;
		}
	}
}

void getCursorTexture() {
	if(!cursorTexture) {
		glGenTextures(1, &cursorTexture);
	}

	XFixesCursorImage *cursor_image;
	cursor_image = XFixesGetCursorImage (dpy);

	//Annoyingly, xfixes specifies the data to be 32bit, but places it in an unsigned long *
	//which can be 64 bit.  So we need to iterate over a 64bit structure to put it in a 32bit
	//structure.
//	std::cerr << cursor_image->width << "x" << cursor_image->height << std::endl;
	GLuint *pixels = new GLuint[cursor_image->width * cursor_image->height];
	for (int i = 0; i < cursor_image->width * cursor_image->height; ++i) {
		pixels[i] = cursor_image->pixels[i] & 0xffffffff;
		pixels[i] = (pixels[i]>>24 & 0x000000FF) ? (pixels[i]|0xFF000000) : pixels[i];
	}

	glBindTexture(GL_TEXTURE_2D, cursorTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cursor_image->width, cursor_image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);//cursor_image->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	  //cursor_image->xhot;
	  //cursor_image->yhot;

	delete[] pixels;

	XFree(cursor_image);
}

static Bool WaitForNotify(Display *d, XEvent *e, char *arg) {
    return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}
int main(int argc, char ** argv)
{
	prep_root();

    XEvent event;
    Bool done = False;

    XWindowAttributes attr;
    XGetWindowAttributes( dpy, root, &attr );
    width = attr.width;
    height = attr.height;

    createWindow();
    XIfEvent(dpy, &event, WaitForNotify, (char*)window);

    prep_overlay();
    prep_stage();
//    prep_input();
//    prep_input2();
    XIfEvent(dpy, &event, WaitForNotify, (char*)overlay);
    
    setup_iphone_listener();

    glutInit(&argc, argv);

    if(texture == 0) {
    	std::cerr << "gen texture" << std::endl;
    	glGenTextures (1, &texture);
//    	texture = LoadTextureRAW("texture.raw", false);
    	CheckForErrors();
    	std::cerr << "gen texture done" << std::endl;
    }

    if(USE_FBO) prep_framebuffers();
    std::cerr << "A" << std::endl;


    XSync(dpy, false);
    std::cerr << "*********** HERE" << std::endl;

	static GLXContext ctx = glXGetCurrentContext();
	static GLXDrawable d = glXGetCurrentDrawable();
	glXMakeContextCurrent(dpy, d, d, ctx);

	std::cerr << "dpy: " << dpy << ", display: " << display << std::endl;

	// set X error handler here since expected errors on some window mappings
	XSetErrorHandler(errorHandler);

	loadSkybox();


	XFixesHideCursor (dpy, overlay);
	setup_hotkey(dpy);

	loadModel();

	struct timespec ts_start;
	clock_gettime(CLOCK_MONOTONIC, &ts_start);

    XEvent peekEvent;
    int pointerX, pointerY;
    unsigned int pointerMods;
    // wait for events and eat up cpu. ;-)
    while (!done)
    {
        // handle the events in the queue
        while (XPending(dpy) > 0)
        {
        	// std::cerr << XPending(dpy) << std::endl;
            XNextEvent(dpy, &event);
            // std::cerr << "Event: " << event.type << std::endl;

            XGenericEventCookie *cookie = &event.xcookie;
            if(event.xcookie.extension==xi_opcode && event.xcookie.type == GenericEvent) {
            	XGetEventData(dpy, cookie);
            	XIDeviceEvent *xi_event = (XIDeviceEvent*)event.xcookie.data;
//				printf("XI EVENT type %d\n", xi_event->evtype);
				switch (xi_event->evtype)
				{
				case XI_KeyPress:
					if(!controlDesktop) {
						processKey(xi_event, true);
					}
					break;
				case XI_KeyRelease:
//					std::cerr << "RELEASE" << std::endl;
					if(!controlDesktop) {
						processKey(xi_event, false);
					}
					break;
				case XI_RawMotion:
					if(!controlDesktop) {
						process_rawmotion((XIRawEvent*)event.xcookie.data);
					}
					break;
				}

				XFreeEventData(dpy, cookie);
				continue;
            }
            if (event.xany.type == xfixes_event_base + XFixesCursorNotify) {
				XFixesCursorNotifyEvent *notify_event = (XFixesCursorNotifyEvent *)(&event);
				if (notify_event->subtype == XFixesDisplayCursorNotify) {
//					std::cerr << "CURSOR NOTIFY" << std::endl;
					getCursorTexture();
				}
				continue;
            }
            switch (event.type)
            {
//            case MapNotify:
////            	std::cerr << "MapNotify" << std::endl;
//            	break;
            case Expose:
            case ConfigureNotify:
//            	std::cerr << "configureNotify" << std::endl;
            case MapNotify:
//            	std::cerr << "MapNotify" << std::endl;
            case DestroyNotify:
//            	std::cerr << "DestroyNotify" << std::endl;
            case UnmapNotify:
//				std::cerr << "UnmapNotify" << std::endl;
            	unbindRedirectedWindow(event.xclient.window);
				break;
            case ButtonPress:
            	case ButtonRelease:
            	    pointerX = event.xbutton.x_root;
            	    pointerY = event.xbutton.y_root;
            	    pointerMods = event.xbutton.state;
            	    break;
            	case KeyPress:
            	case KeyRelease:
//            		std::cerr << "KEY PreSS" << std::endl;
            	    pointerX = event.xkey.x_root;
            	    pointerY = event.xkey.y_root;
            	    pointerMods = event.xbutton.state;
            	    if(event.type == KeyRelease) {
            	    	processKey(event.xkey);
            	    }
            	    break;
            	case MotionNotify:
//            		std::cerr << "MOTION" << std::endl;
            	    while (XPending (dpy))
            	    {
						XPeekEvent (dpy, &peekEvent);

						if (peekEvent.type != MotionNotify)
							break;

						XNextEvent (dpy, &event);
            	    }

            	    pointerX = event.xmotion.x_root;
            	    pointerY = event.xmotion.y_root;
            	    pointerMods = event.xbutton.state;

//            		std::cerr << "MOUSE MOVED" << ", " << pointerX << ", " << pointerY << ", event.xmotion.x: " << event.xmotion.x << ", event.xmotion.y: " << event.xmotion.y << std::endl;
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

        if(controlDesktop) {
        	walkForward = strafeRight = 0;
        }
        walk(walkForward, strafeRight, (double)(ts_current.tv_sec-ts_start.tv_sec)+((double)(ts_current.tv_nsec-ts_start.tv_nsec)/(double)1.0e9));

        renderGL();

        ts_start = ts_current;
    }

    destroyWindow();

    return 0;
}
