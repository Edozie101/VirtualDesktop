#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/XEVI.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XInput2.h>
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

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>

#define WIDTH  (1280.0)
#define HEIGHT (800.0)
//#define HEIGHT (WIDTH*900./1440.)
#define TITLE "HeshWM"

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

GLWindow GLWin;

/* most important variable
 * it contains information about the X server which we communicate with
 */
Display               * display;
int                     screen;
/* our window instance */
Window                  window;
GLXContext              context;
XSetWindowAttributes    winAttr;
Bool                    fullscreen = True;
Bool                    doubleBuffered;
/* original desktop mode which we save so we can restore it later */
XF86VidModeModeInfo     desktopMode;
int                     x, y;
unsigned int            width, height;
unsigned int            depth;

GLfloat                 rotQuad = 0.0f;

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

/* prototypes */
void createWindow();
void destroyWindow();
void resizeGL(unsigned int, unsigned int);
void initGL();

void createWindow()
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
    display = XOpenDisplay(0);
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
        XF86VidModeSwitchToMode(display, screen, modes[bestMode]);
        XF86VidModeSetViewPort(display, screen, 0, 0);
        dpyWidth = modes[bestMode]->hdisplay;
        dpyHeight = modes[bestMode]->vdisplay;
        printf("resolution %dx%d\n", dpyWidth, dpyHeight);
        XFree(modes);

        /* set window attributes */
        winAttr.override_redirect = True;
        winAttr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
            StructureNotifyMask | SubstructureNotifyMask;// | PointerMotionMask;
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
            StructureNotifyMask | SubstructureNotifyMask;
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
    gluPerspective(45.0f, 1, 0.1f, 100.0f);//(GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
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
