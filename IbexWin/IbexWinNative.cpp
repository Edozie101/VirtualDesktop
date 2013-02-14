// IbexWinNative.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "IbexWinNative.h"

#include <condition_variable>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

#include <stdio.h>

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
#include <GL/freeglut.h>

#include "ibex_win_utils.h"

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

#include "math_3d.h"

#include "ibex.h"

bool modifiedDesktop(false);
GLuint VBO(0);
std::condition_variable screenshotCondition;

void Keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
  case 'w':
	  walkForward = 1;
	  break;
  case 'a':
  case 'q':
	  strafeRight = -1;
	  break;
  case 's':
	  walkForward = -1;
	  break;
  case 'd':
  case 'e':
	  strafeRight = 1;
	  break;
  }
}
void KeyboardUp(unsigned char key, int x, int y)
{
  switch (key)
  {
  case 'b':
    barrelDistort = !barrelDistort;
    break;
  case 'g':
	  showGround = !showGround;
	  break;
  case 'r':
	  resetPosition = 1;
	  break;
  case 'w':
	  walkForward = 0;
	  break;
  case 'a':
  case 'q':
	  strafeRight = 0;
	  break;
  case 's':
	  walkForward = 0;
	  break;
  case 'd':
  case 'e':
	  strafeRight = 0;
	  break;
  }
}

void MouseMoved(int x, int y) {
	if(!controlDesktop) {
		relativeMouseX = x-500;
		relativeMouseY = y-500;
	}
//    NSLog(@"%f, %f", relativeMouseX, relativeMouseY);
}

static inline void getMouseCursor(HDC hdcScreen)
{
	CURSORINFO cursorinfo = { 0 };
	cursorinfo.cbSize = sizeof(cursorinfo);
	if(GetCursorInfo(&cursorinfo))  {
		ICONINFO ii = {0};
		if(GetIconInfo(cursorinfo.hCursor, &ii)) {
			BITMAP bitmap = {0};
			GetObject(ii.hbmColor, sizeof(bitmap), &bitmap);

			int w = ((bitmap.bmWidth * bitmap.bmBitsPixel) + 31) / 8;
			static BYTE* bits = new BYTE[w * bitmap.bmHeight];
			memset(bits, 0, sizeof(bits));

			BITMAPINFO bmi;
			memset(&bmi, 0, sizeof(BITMAPINFO)); 
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth =  bitmap.bmWidth;
			bmi.bmiHeader.biHeight =  bitmap.bmHeight;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biCompression = BI_RGB;
			int rv = ::GetDIBits(hdcScreen, ii.hbmColor, 0, bitmap.bmHeight, bits, (BITMAPINFO *)&bmi, DIB_RGB_COLORS);

			if(cursor == 0) {
				glGenTextures(1, &cursor);
			}
			glBindTexture(GL_TEXTURE_2D, cursor);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bitmap.bmWidth, bitmap.bmHeight, 0,
				GL_BGRA, GL_UNSIGNED_BYTE, bits);
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glBindTexture(GL_TEXTURE_2D, 0);

			//delete []bits;
		}
	}
}

int CaptureAnImage(HWND hWnd)
{
    HDC hdcScreen;
    HDC hdcWindow;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;

    // Retrieve the handle to a display device context for the client 
    // area of the window. 
    hdcScreen = GetDC(NULL);
    hdcWindow = GetDC(hWnd);

    // Create a compatible DC which is used in a BitBlt from the window DC
    hdcMemDC = CreateCompatibleDC(hdcWindow); 

    if(!hdcMemDC)
    {
        MessageBox(hWnd, L"CreateCompatibleDC has failed",L"Failed", MB_OK);
        goto done;
    }

    // Get the client area for size calculation
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);
    
    // Create a compatible bitmap from the Window DC
    hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top);
    
    if(!hbmScreen)
    {
        MessageBox(hWnd, L"CreateCompatibleBitmap Failed",L"Failed", MB_OK);
        goto done;
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC,hbmScreen);
    
    // Bit block transfer into our compatible memory DC.
    if(!BitBlt(hdcMemDC, 
               0,0, 
               rcClient.right-rcClient.left, rcClient.bottom-rcClient.top, 
               hdcWindow, 
               0,0,
               SRCCOPY))
    {
        MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    // Get the BITMAP from the HBITMAP
    GetObject(hbmScreen,sizeof(BITMAP),&bmpScreen);
     
    BITMAPFILEHEADER   bmfHeader;    
    BITMAPINFOHEADER   bi;
     
    bi.biSize = sizeof(BITMAPINFOHEADER);    
    bi.biWidth = bmpScreen.bmWidth;    
    bi.biHeight = bmpScreen.bmHeight;  
    bi.biPlanes = 1;    
    bi.biBitCount = 32;    
    bi.biCompression = BI_RGB;    
    bi.biSizeImage = 0;  
    bi.biXPelsPerMeter = 0;    
    bi.biYPelsPerMeter = 0;    
    bi.biClrUsed = 0;    
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.
    //HANDLE hDIB = GlobalAlloc(GHND,dwBmpSize); 
    //char *lpbitmap = (char *)GlobalLock(hDIB);    
	static char *lpbitmap = new char[dwBmpSize];

    // Gets the "bits" from the bitmap and copies them into a buffer 
    // which is pointed to by lpbitmap.
    GetDIBits(hdcWindow, hbmScreen, 0,
				(UINT)bmpScreen.bmHeight,
				lpbitmap,
				(BITMAPINFO *)&bi, DIB_RGB_COLORS);

		glBindTexture(GL_TEXTURE_2D, desktopTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bmpScreen.bmWidth, bmpScreen.bmHeight, 0,
               GL_BGRA, GL_UNSIGNED_BYTE, lpbitmap);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glBindTexture(GL_TEXTURE_2D, 0);

	// free []lpbitmap;

    //Unlock and Free the DIB from the heap
    //GlobalUnlock(hDIB);    
    //GlobalFree(hDIB);

	getMouseCursor(hdcScreen);

    //Clean up
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL,hdcScreen);
    ReleaseDC(hWnd,hdcWindow);

	return 0;
}

void getScreenshot() {
	HWND hwnd = GetDesktopWindow();
	CaptureAnImage(hwnd);
}

HGLRC loaderContext;
HDC hdc;
void loopScreenshot() {
	std::mutex screenshotMutex;
	std::unique_lock<std::mutex> screenshotLock(screenshotMutex);
	wglMakeCurrent(hdc, loaderContext);
	while(1) {
		screenshotCondition.wait(screenshotLock);
		getScreenshot();
	}
}

Ibex *ibex = 0;
static void RenderSceneCB()
{
	static double timeprev = glutGet(GLUT_ELAPSED_TIME);
	double time = glutGet(GLUT_ELAPSED_TIME);
	double timeDiff = (time - timeprev)/1000.0;
	timeprev = time;

	//static double timebase = glutGet(GLUT_ELAPSED_TIME);
	//static double frame = 0;
	//++frame;
	//static char fpsString[64];
	//if (time - timebase >= 5000.0) {
	//	sprintf(fpsString,"FPS:%4.2f", frame*5000.0/(time-timebase));
	//	timebase = time;
	//	frame = 0;
	//}

	if(modifiedDesktop) {
		modifiedDesktop = false;
		if(controlDesktop) {
			glutSetCursor(GLUT_CURSOR_INHERIT);
		} else {
			glutSetCursor(GLUT_CURSOR_NONE);
		}
	}

	if(!controlDesktop) {
		glutWarpPointer(500, 500);
	}

	// Add your drawing codes here
    if(ibex == 0) {
		char *argv[] = {""};
        ibex = new Ibex(0,0);
    }
    
	POINT p;
	if (GetCursorPos(&p))
	{
		cursorPosX = p.x;
		cursorPosY = physicalHeight-p.y;
	}

    ibex->render(timeDiff);

	glutSwapBuffers();
	glutPostRedisplay();

	screenshotCondition.notify_all();
}

void ReshapeFunc(int width, int height) {
	resizeGL(width, height);
}

static void InitializeGlutCallbacks()
{
	glutWarpPointer(500, 500);

    glutDisplayFunc(RenderSceneCB);
	glutKeyboardFunc (Keyboard);
	glutKeyboardUpFunc (KeyboardUp);
	glutPassiveMotionFunc (MouseMoved);
	glutReshapeFunc(ReshapeFunc);
}

HWND hwnd;
DWORD WINAPI mainThreadId;
void globalHotkeyListener() {
	//AttachThreadInput(mainThreadId,GetCurrentThreadId(),true);

	RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'g');
	RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'G');

	MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        //TranslateMessage(&msg);
        //DispatchMessage(&msg);
		if(msg.message == WM_HOTKEY) {
			controlDesktop = !controlDesktop;
			modifiedDesktop = true;
			if(controlDesktop) {
			} else {
				SetForegroundWindow (hwnd);
				SetActiveWindow(hwnd);
				SetFocus(hwnd);
			}
		}
    }
	return;
}
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	width = 1280;
	height = 800;
	physicalWidth = 1920;//(false) ? 1920 : width;
	physicalHeight = 1080;//(false) ? 1080 : height;
	windowWidth = 1280;
	windowHeight = 800;

	controlDesktop = 0;
	modifiedDesktop = true;

	size_t len = 0;
	wchar_t cwd[1024];
	GetCurrentDirectory(1023, cwd);
	wcstombs_s(&len, mResourcePath, cwd, 1023);

	int argc = 0;
	char **argv = 0;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Ibex");

    InitializeGlutCallbacks();

    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }

	hwnd = GetActiveWindow();
	hdc = GetDC(hwnd);
	HGLRC mainContext = wglGetCurrentContext();
    loaderContext = wglCreateContext(hdc);
    wglShareLists(loaderContext, mainContext); // Order matters
	mainThreadId = GetCurrentThreadId();

	std::thread screenshotThread(loopScreenshot);
	std::thread hotkeyThread(globalHotkeyListener);

	SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED);

    glutMainLoop();

	screenshotThread.join();
	hotkeyThread.join();

	return 0;
}
