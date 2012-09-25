/*
 * opengl_setup_x11.cpp
 *
 *  Created on: Sep 22, 2012
 *      Author: parallels
 */
#include <iostream>

#include "opengl_setup_x11.h"

GLXFBConfig fbconfig;
bool glxYInverted;

Display               * display;
int                     screen;
Window                  window;
GLXContext              context;
XF86VidModeModeInfo     desktopMode; // original desktop mode which we save so we can restore it later
Bool                    doubleBuffered;
unsigned int            width, height;

XVisualInfo *visinfo;


/* most important variable
 * it contains information about the X server which we communicate with
 */
static GLWindow GLWin;
static XSetWindowAttributes    winAttr;
static Bool                    fullscreen = True;
//static int                     x, y;
//static unsigned int            depth;


/* attributes for a single buffered visual in RGBA format with at least
 * 4 bits per color and a 16 bit depth buffer */
static int attrListSgl[] =
{
    GLX_RGBA, GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 16,
    None
};

/* attributes for a double buffered visual in RGBA format with at least
 * 4 bits per color and a 16 bit depth buffer */
static int attrListDbl[] =
{
    GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 16,
    None
};


void initFBConfig(Display *dpy, Window root)
{
  XWindowAttributes attr;
  int value;
  int nfbconfigs;
  GLXFBConfig *fbconfigs = glXGetFBConfigs(display, screen, &nfbconfigs);
  int i = 0;
  XGetWindowAttributes(dpy, root, &attr);

  int attrib[] =
    { GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1, GLX_ALPHA_SIZE, 1,
        GLX_DOUBLEBUFFER, True, GLX_DEPTH_SIZE, 1, None };

  int numfbconfigs, render_event_base, render_error_base;
  XRenderPictFormat *pictFormat;

  // Make sure we have the RENDER extension
  if (!XRenderQueryExtension(dpy, &render_event_base, &render_error_base)) {
    std::cerr << "No RENDER extension found" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Get the list of FBConfigs that match our criteria
  int scrnum = 0;
  fbconfigs = glXChooseFBConfig(dpy, scrnum, attrib, &numfbconfigs);
  if (!fbconfigs) {
    // None matched
    exit(EXIT_FAILURE);
  }

  // Find an FBConfig with a visual that has a RENDER picture format that
  // has alpha
  for (i = 0; i < numfbconfigs; i++) {
    visinfo = glXGetVisualFromFBConfig(dpy, fbconfigs[i]);
    if (!visinfo) continue;
    pictFormat = XRenderFindVisualFormat(dpy, visinfo->visual);
    if (!pictFormat) continue;

    if (pictFormat->direct.alphaMask > 0) {
      fbconfig = fbconfigs[i];
      break;
    }

    XFree(visinfo);
  }

  if (i == numfbconfigs) {
    // None of the FBConfigs have alpha.  Use a normal (opaque)
    // FBConfig instead
    fbconfig = fbconfigs[0];
    visinfo = glXGetVisualFromFBConfig(dpy, fbconfig);
    pictFormat = XRenderFindVisualFormat(dpy, visinfo->visual);
  }

  glXGetFBConfigAttrib(dpy, fbconfigs[i], GLX_Y_INVERTED_EXT, &value);
  glxYInverted = value;

  glEnable(GL_TEXTURE_2D);

  fbconfig = fbconfigs[i];

  XFree(fbconfigs);
}

void createWindow(Display *dpy_, Window root_)
{
    XVisualInfo *vi;
    Colormap cmap;
    int i, dpyWidth, dpyHeight;
    int glxMajor, glxMinor, vmMajor, vmMinor;
    XF86VidModeModeInfo **modes;
    int modeNum, bestMode;
    Atom wmDelete;
//    Window winDummy;
//    unsigned int borderDummy;

    /* set best mode to current */
    bestMode = 0;
    /* get a connection */
    display = dpy_;
    screen = DefaultScreen(display);
    XF86VidModeQueryVersion(display, &vmMajor, &vmMinor);
    printf("XF86 VideoMode extension version %d.%d\n", vmMajor, vmMinor);
    XF86VidModeGetAllModeLines(display, screen, &modeNum, &modes);
    /* save desktop-resolution before switching modes */
    GLWin.deskMode = *modes[0];
    desktopMode = *modes[0];
    /* look for mode with requested resolution */
    for (i = 0; i < modeNum; i++)
    {
        if ((modes[i]->hdisplay == width) && (modes[i]->vdisplay == height))
            bestMode = i;
    }
    /* get an appropriate visual */
    vi = glXChooseVisual(display, screen, attrListDbl);
    if (vi == NULL)
    {
        vi = glXChooseVisual(display, screen, attrListSgl);
        doubleBuffered = False;
        printf("singlebuffered rendering will be used, no doublebuffering available\n");
    }
    else
    {
        doubleBuffered = True;
        printf("doublebuffered rendering available\n");
    }
    glXQueryVersion(display, &glxMajor, &glxMinor);
    printf("GLX-Version %d.%d\n", glxMajor, glxMinor);
    /* create a GLX context */
    context = glXCreateContext(display, vi, 0, GL_TRUE);
    /* create a color map */
    cmap = XCreateColormap(display, RootWindow(display, vi->screen),
        vi->visual, AllocNone);
    winAttr.colormap = cmap;
    winAttr.border_pixel = 0;

    if (fullscreen)
    {
        /* switch to fullscreen */
        XSync(display, false);
        std::cerr << "Best Mode: " << bestMode << ", " <<  modes[bestMode]->hdisplay << "x" <<  modes[bestMode]->vdisplay << std::endl;
        Bool switched = XF86VidModeSwitchToMode(display, screen, modes[bestMode]);
        XSync(display, false);
        if(switched == False) {
                std::cerr << "Couldn't switch to resolution: " << WIDTH << ", " << HEIGHT << std::endl;
                exit(1);
        }
        XF86VidModeSetViewPort(display, screen, 0, 0);
        dpyWidth = modes[bestMode]->hdisplay;
        dpyHeight = modes[bestMode]->vdisplay;
        printf("resolution %dx%d\n", dpyWidth, dpyHeight);
        XFree(modes);

        /* set window attributes */
        winAttr.override_redirect = True;
        winAttr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
            SubstructureNotifyMask;// | SubstructureNotifyMask;// | PointerMotionMask;
        window = XCreateWindow(display, RootWindow(display, vi->screen),
            0, 0, dpyWidth, dpyHeight, 0, vi->depth, InputOutput, vi->visual,
            CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect,
            &winAttr);
        XWarpPointer(display, None, window, 0, 0, 0, 0, 0, 0);
                XMapRaised(display, window);
        XGrabKeyboard(display, window, True, GrabModeAsync,
            GrabModeAsync, CurrentTime);
        XGrabPointer(display, window, True, ButtonPressMask,
            GrabModeAsync, GrabModeAsync, window, None, CurrentTime);
    }
    else
    {
        /* create a window in window mode*/
        winAttr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
            SubstructureNotifyMask;// | SubstructureNotifyMask;
        window = XCreateWindow(display, RootWindow(display, vi->screen),
            0, 0, width, height, 0, vi->depth, InputOutput, vi->visual,
            CWBorderPixel | CWColormap | CWEventMask, &winAttr);
        /* only set window title and handle wm_delete_events if in windowed mode */
        wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);
        XSetWMProtocols(display, window, &wmDelete, 1);
        XSetStandardProperties(display, window, TITLE,
            TITLE, None, NULL, 0, NULL);
        XMapRaised(display, window);
    }

    std::cerr << "XF86" << std::endl;

    /* connect the glx-context to the window */
    bool r = glXMakeCurrent(display, window, context);
    std::cerr << "Make current success? " << r << std::endl;
    if (glXIsDirect(display, context)) {
        printf("DRI enabled\n");
    } else
        printf("no DRI available\n");

    glewExperimental = true;
    GLenum err = glewInit();
    std::cerr << "Inited GLEW: " << err << std::endl;
    if (GLEW_OK != err)
    {
        // GLEW failed!
        exit(1);
    }

    XSync(display, false);
    initFBConfig(display, root_);

    initGL();
}

/*
 * destroy the window
 */
void destroyWindow()
{
    if( context )
    {
        if( !glXMakeCurrent(display, None, NULL))
        {
            printf("Could not release drawing context.\n");
        }
        /* destroy the context */
        glXDestroyContext(display, context);
        context = NULL;
    }
    /* switch back to original desktop resolution if we were in fullscreen */
    if( fullscreen )
    {
        XF86VidModeSwitchToMode(display, screen, &desktopMode);
        XF86VidModeSetViewPort(display, screen, 0, 0);
    }
    XCloseDisplay(display);
}

void resizeGL(unsigned int width, unsigned int height)
{
    /* prevent divide-by-zero */
    if (height == 0)
        height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
//    gluPerspective(45.0f, 1, 0.1f, 100.0f);//(GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    gluPerspective(110.0f, 0.81818181, 0.01f, 1000.0f);
//    gluPerspective(120.0f, 0.75, 0.1f, 100.0f);//(GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

// load a 256x256 RGB .RAW file as a texture
GLuint LoadTextureRAW( const char * filename, int wrap )
{
    GLuint texture;
    int width, height;
    BYTE * data;
    FILE * file;

    // open texture data
    file = fopen( filename, "rb" );
    if ( file == NULL ) return 0;

    // allocate buffer
    width = 256;
    height = 256;
    data = (BYTE*)malloc( width * height * 3 );

    // read texture data
    fread( data, width * height * 3, 1, file );
    fclose( file );

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the first mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     wrap ? GL_REPEAT : GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                     wrap ? GL_REPEAT : GL_CLAMP );

    // build our texture mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height,
                       GL_RGB, GL_UNSIGNED_BYTE, data );

    // free buffer
    free( data );

    return texture;
}

//-------------------------------------------------------------------------------------------------//
GLXFBConfig*
chooseFBConfig(Display *dpy, const GLint *attribList, GLint *nElements)
{
  GLXFBConfig *fbConfigs;

  if (GLXEW_VERSION_1_3)
    fbConfigs = glXChooseFBConfig(dpy, DefaultScreen(dpy), attribList,
        nElements);
  else
    fbConfigs = glXChooseFBConfigSGIX(dpy, DefaultScreen(dpy), attribList,
        nElements);

  return fbConfigs;
}

//-------------------------------------------------------------------------------------------------//
XVisualInfo*
getVisualFromFBConfig(Display *dpy, GLXFBConfig fbConfig)
{
  XVisualInfo *visualInfo;

  if (GLXEW_VERSION_1_3)
    visualInfo = glXGetVisualFromFBConfig(dpy, fbConfig);
  else
    visualInfo = glXGetVisualFromFBConfigSGIX(dpy, fbConfig);

  return visualInfo;
}

//-------------------------------------------------------------------------------------------------//
// Finds a GLXFBConfig compatible with a given VisualID.
GLXFBConfig
getFBConfigFromVisualID(Display *dpy, VisualID visualid)
{
  GLXFBConfig fbConfig = 0;

  if (GLXEW_SGIX_fbconfig && glXGetFBConfigFromVisualSGIX) {
    XVisualInfo visualInfo;

    visualInfo.screen = DefaultScreen(dpy);
    visualInfo.depth = DefaultDepth(dpy, DefaultScreen(dpy));
    visualInfo.visualid = visualid;

    fbConfig = glXGetFBConfigFromVisualSGIX(dpy, &visualInfo);
  }

  if (!fbConfig) {
    int minAttribs[] =
      { GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT || GLX_PIXMAP_BIT, GLX_RENDER_TYPE,
          GLX_RGBA_BIT, GLX_RED_SIZE, 1, GLX_BLUE_SIZE, 1, GLX_GREEN_SIZE, 1,
          None };
    int nConfigs = 0;

    GLXFBConfig *fbConfigs = chooseFBConfig(dpy, minAttribs, &nConfigs);

    for (int i = 0; i < nConfigs && !fbConfig; i++) {
      XVisualInfo *visualInfo = getVisualFromFBConfig(dpy, fbConfigs[i]);

      if (visualInfo->visualid == visualid) fbConfig = fbConfigs[i];

      XFree(visualInfo);
    }

    XFree(fbConfigs);
  }

  return fbConfig;
}

//-------------------------------------------------------------------------------------------------//
// Returns the FBConfig behind a GLXContext
GLXFBConfig
getFBConfigFromContext(Display *dpy, GLXContext context)
{
  GLXFBConfig fbConfig = 0;

  if (GLXEW_VERSION_1_3) {
    int fbConfigAttrib[] =
      { GLX_FBCONFIG_ID, 0, None };
    GLXFBConfig *fbConfigs;
    int nElements = 0;

    glXQueryContext(dpy, context, GLX_FBCONFIG_ID, &fbConfigAttrib[1]);
    fbConfigs = glXChooseFBConfig(dpy, DefaultScreen(dpy), fbConfigAttrib,
        &nElements);

    if (nElements) {
      fbConfig = fbConfigs[0];
      XFree(fbConfigs);
    }
  }
  else if (GLXEW_EXT_import_context && GLXEW_SGIX_fbconfig) {
    VisualID visualid;

    if (glXQueryContextInfoEXT(dpy, context, GLX_VISUAL_ID, (int*) &visualid)) {
      fbConfig = getFBConfigFromVisualID(dpy, visualid);
    }
  }

  return fbConfig;
}
