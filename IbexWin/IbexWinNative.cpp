// IbexWinNative.cpp : Defines the entry point for the application.
//

#ifdef _WIN32
#include "video/VideoPlayer.h"
#endif

#include "stdafx.h"

#include "IbexWinNative.h"
#include "opengl_helpers.h"

#if _USE_SIXENSE
#include "sixense_controller.h"
#endif

// add if you will call: dwmapi.lib
// #include <Dwmapi.h>

#include <condition_variable>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>

#include <string.h>

#include "guicon.h"
#include <crtdbg.h>

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

#include "video/VideoPlayer.h"

#include "GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glext.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "ibex_win_utils.h"
#include "opengl_helpers.h"

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
#include "RendererPlugin.h"

#include "distortions.h"
#include "OVR.h"

OVR::Ptr<OVR::DeviceManager>	pManager;
OVR::Ptr<OVR::HMDDevice>		pHMD;
OVR::Ptr<OVR::SensorDevice>	pSensor;
OVR::SensorFusion		FusionResult;
OVR::HMDInfo				Info;
bool				InfoLoaded = false;
bool				riftConnected = false;

bool modifiedDesktop(false);
GLuint VBO(0);
std::condition_variable screenshotCondition;

Ibex::Ibex *ibex = 0;

static inline void processSpecialKey(int key, int down) {
	if(showDialog) ibex->renderer->window.processSpecialKey(key, down);
	else {
		switch(key) {
        case GLUT_KEY_UP:
			walkForward = 1 * down;
            break;
        case GLUT_KEY_DOWN:
            walkForward = -1 * down;
            break;
		case GLUT_KEY_RIGHT:
            strafeRight = 1 * down;
            break;
		case GLUT_KEY_LEFT:
            strafeRight = -1 * down;
            break;
		}
	}
}
static void processSpecialKeyDown(int key, int x, int y) {
	processSpecialKey(key, 1);
}
static void processSpecialKeyUp(int key, int x, int y) {
	processSpecialKey(key, 0);
}
static inline void Keyboard(unsigned char key, int down) {
	int processed = 0;
	if(showDialog) {
		processed = ibex->renderer->window.processKey(key, down);
	}
	if(!processed) {
		switch (key)
		{
		case 'w':
		case 'W':
			walkForward = 1*down;
			break;
		case 'a':
		case 'A':
			strafeRight = -1*down;
			break;
		case 's':
		case 'S':
			walkForward = -1*down;
			break;
		case 'd':
		case 'D':
			strafeRight = 1*down;
			break;
		case '=':
		case '+':
			if(down) {
				IOD += 0.0005;
				lensParametersChanged = true;
			}
			break;
		case 'Q':
		case 'q':
			if(down) {
				if(glutGetModifiers() == GLUT_ACTIVE_CTRL) {
					exit(0); 
					break; 
				}
				displayShape = (displayShape == FlatDisplay) ? SphericalDisplay : FlatDisplay;
			}
			break;
		case '/':
			if(down) {
				showDialog = !showDialog;
				ibex->renderer->window.reset();
			}
			break;
		case '-':
		case '_':
			if(down) {
				IOD -= 0.0005;
				lensParametersChanged = true;
			}
			break;
		case 'b':
		case 'B':
			if(!down) {
				barrelDistort = !barrelDistort;
			}
			break;
		case 'g':
		case 'G':
			if(!down) {
				showGround = !showGround;
			}
			break;
		case 'r':
		case 'R':
			if(!down) {
				resetPosition = 1;
			}
			break;
		}
	}
}
static void KeyboardDown(unsigned char key, int x, int y)
{
	Keyboard(key, 1);
}
static void KeyboardUp(unsigned char key, int x, int y)
{
	Keyboard(key, 0);
}

static inline void MouseMoved(int x, int y) {
	if(!controlDesktop) {
		relativeMouseX = x-500;
		relativeMouseY = y-500;
	}
//    NSLog(@"%f, %f", relativeMouseX, relativeMouseY);
}

static inline void getMouseCursor(HDC hdcScreen)
{
	static CURSORINFO cursorinfo = { 0 };
	static HCURSOR prevCursor = 0;
	cursorinfo.cbSize = sizeof(cursorinfo);
	if(GetCursorInfo(&cursorinfo) && cursorinfo.hCursor != prevCursor)  {
		prevCursor = cursorinfo.hCursor;
		ICONINFO ii = {0};
		if(GetIconInfo(cursorinfo.hCursor, &ii)) {
			BITMAP bitmap = {0};
			if(ii.hbmColor != 0) {
				mouseBlendAlternate = false;
				GetObject(ii.hbmColor, sizeof(bitmap), &bitmap);

				int w = ((bitmap.bmWidth * bitmap.bmBitsPixel) + 31) / 8;
				static BYTE* bits = new BYTE[w * bitmap.bmHeight];
				static int size = w * bitmap.bmHeight;
				memset(bits, 0, size);//sizeof(bits));

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
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				//glBindTexture(GL_TEXTURE_2D, 0);

				//delete []bits;
			} else if(ii.hbmMask != 0) {
				mouseBlendAlternate = true;
				GetObject(ii.hbmMask, sizeof(bitmap), &bitmap);

				int w = ((bitmap.bmWidth * /*bitmap.bmBitsPixel*/ 32) + 31) / 8;
				static BYTE *bits = new BYTE[w * bitmap.bmHeight];
				static int size = w * bitmap.bmHeight;
				memset(bits, 0, size);//sizeof(bits));

				BITMAPV5HEADER bmi;
				memset(&bmi, 0, sizeof(BITMAPV5HEADER)); 
				bmi.bV5Size = sizeof(BITMAPV5HEADER);
				bmi.bV5Width =  bitmap.bmWidth;
				bmi.bV5Height =  bitmap.bmHeight;
				bmi.bV5BitCount = 32;
				bmi.bV5Planes = 1;
				bmi.bV5Compression = BI_BITFIELDS; //BI_RGB;
				bmi.bV5RedMask   =  0x00FF0000;
				bmi.bV5GreenMask =  0x0000FF00;
				bmi.bV5BlueMask  =  0x000000FF;
				bmi.bV5AlphaMask =  0xFF000000;
				int rv = ::GetDIBits(hdcScreen, ii.hbmMask, 0, bitmap.bmHeight/2, bits, (BITMAPINFO *)&bmi, DIB_RGB_COLORS);

				if(cursor == 0) {
					glGenTextures(1, &cursor);
				}
				glBindTexture(GL_TEXTURE_2D, cursor);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bitmap.bmWidth, bitmap.bmHeight/2, 0,
					GL_BGRA, GL_UNSIGNED_BYTE, bits);
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				//glBindTexture(GL_TEXTURE_2D, 0);

				//delete []bits;
			}
		}
	}
}

static inline int CaptureAnImage(HWND hWnd)
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

    // Get the BITMAP from the HBITMAP
    GetObject(hbmScreen,sizeof(BITMAP),&bmpScreen);
     
    BITMAPFILEHEADER   bmfHeader;    
    BITMAPINFOHEADER   bi;
     
    bi.biSize = sizeof(BITMAPINFOHEADER);    
    bi.biWidth = bmpScreen.bmWidth;    
    bi.biHeight = bmpScreen.bmHeight;  
    bi.biPlanes = 1;    
    bi.biBitCount = 24;    
    bi.biCompression = BI_RGB;    
    bi.biSizeImage = 0;  
    bi.biXPelsPerMeter = 0;    
    bi.biYPelsPerMeter = 0;    
    bi.biClrUsed = 0;    
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 23) / 24) * 3 * bmpScreen.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.
    //HANDLE hDIB = GlobalAlloc(GHND,dwBmpSize); 
    //char *lpbitmap = (char *)GlobalLock(hDIB);    
	static char *lpbitmap = new char[dwBmpSize];

	static std::mutex screenshotMutex;
	static std::unique_lock<std::mutex> screenshotLock(screenshotMutex);
	screenshotCondition.wait(screenshotLock);

	// Bit block transfer into our compatible memory DC.
    if(!BitBlt(hdcMemDC, 
               0,0, 
               rcClient.right-rcClient.left, rcClient.bottom-rcClient.top, 
               hdcWindow, 
               0,0,
               SRCCOPY | CAPTUREBLT))
    {
        MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    // Gets the "bits" from the bitmap and copies them into a buffer 
    // which is pointed to by lpbitmap.
    GetDIBits(hdcWindow, hbmScreen, 0,
				(UINT)bmpScreen.bmHeight,
				lpbitmap,
				(BITMAPINFO *)&bi, DIB_RGB_COLORS);

	if(desktopTexture) {
		static bool used = false;
		glBindTexture(GL_TEXTURE_2D, desktopTexture);
		if(used) {
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bmpScreen.bmWidth, bmpScreen.bmHeight, GL_BGR, GL_UNSIGNED_BYTE, lpbitmap);
		} else {
			used = true;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, bmpScreen.bmWidth, bmpScreen.bmHeight, 0,
               GL_BGR, GL_UNSIGNED_BYTE, lpbitmap);
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
		}
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

		//glBindTexture(GL_TEXTURE_2D, 0);
	}

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

static inline void getScreenshot() {
	HWND hwnd = GetDesktopWindow();
	CaptureAnImage(hwnd);
}

static HGLRC loaderContext;
static HDC hdc;
static bool captureDesktop = true;
static void loopScreenshot() {
	wglMakeCurrent(hdc, loaderContext);
	while(captureDesktop) {
		getScreenshot();

#ifdef _DEBUG
		static double timeprev = glutGet(GLUT_ELAPSED_TIME);
		const double time = glutGet(GLUT_ELAPSED_TIME);
		timeprev = time;

		static double timebase = glutGet(GLUT_ELAPSED_TIME);
		static double frame = 0;
		++frame;
		static char fpsString[64];
		if (time - timebase >= 5000.0) {
			sprintf(fpsString,"FPS:%4.2f", frame*5000.0/(time-timebase));
			std::cerr << "Capture " << fpsString << std::endl;
			timebase = time;
			frame = 0;
		}
#endif
	}
}

HGLRC videoPlayerContext = NULL;
Ibex::VideoPlayer *_ibexVideoPlayer = NULL;
static void playVideo() {
	bool success = wglMakeCurrent(hdc, videoPlayerContext);
	std::cerr << "Video playing wglMakeCurrent: " << success << std::endl;

	_ibexVideoPlayer = new Ibex::VideoPlayer();
	_ibexVideoPlayer->playVideo(ibex->renderer->window.getSelectedVideoPath().c_str(),ibex->renderer->window.getIsStereoVideo());
	//_ibexVideoPlayer->openCamera(ibex->renderer->window.getIsStereoVideo(), -1);
}
static void playCamera() {
	bool success = wglMakeCurrent(hdc, videoPlayerContext);
	std::cerr << "Video playing wglMakeCurrent: " << success << std::endl;

	_ibexVideoPlayer = new Ibex::VideoPlayer();
	_ibexVideoPlayer->openCamera(ibex->renderer->window.getIsStereoVideo(), ibex->renderer->window.getSelectedCameraID());
}

static void RenderSceneCB()
{
	screenshotCondition.notify_all();
	static double timeprev = glutGet(GLUT_ELAPSED_TIME);
	double time = glutGet(GLUT_ELAPSED_TIME);
	double timeDiff = (time - timeprev)/1000.0;
	timeprev = time;

#ifdef _DEBUG
	static double timebase = glutGet(GLUT_ELAPSED_TIME);
	static double frame = 0;
	++frame;
	static char fpsString[64];
	if (time - timebase >= 5000.0) {
		sprintf(fpsString,"FPS:%4.2f", frame*5000.0/(time-timebase));
		std::cerr << fpsString << std::endl;
		timebase = time;
		frame = 0;
	}
#endif

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
        ibex = new Ibex::Ibex(0,0);
    }
    
	POINT p;
	if (GetCursorPos(&p))
	{
		cursorPosX = p.x;
		cursorPosY = physicalHeight-p.y;
	}

	if(ibex != NULL && (ibex->renderer->window.getSelectedVideo() || ibex->renderer->window.getSelectedCamera())) {
		if(ibex->renderer->window.getSelectedVideo()) {
			std::thread videoThread(playVideo);
			videoThread.detach();
        } else {
			std::thread videoThread(playCamera);
			videoThread.detach();
        }
        ibex->renderer->window.setSelectedVideo(false);
        ibex->renderer->window.setSelectedCamera(false);
	}
	if(_ibexVideoPlayer != NULL) {
		videoTexture[0] = _ibexVideoPlayer->videoTexture[0];
		videoTexture[1] = _ibexVideoPlayer->videoTexture[1];
		videoWidth = _ibexVideoPlayer->width;
		videoHeight = _ibexVideoPlayer->height;
	} else {
		videoWidth = videoHeight = videoTexture[0] = videoTexture[1] = 0;
	}

#if _USE_SIXENSE
	mySixenseRefresh();
#endif

    ibex->render(timeDiff);
	//checkForErrors();

	glutSwapBuffers();
	glutPostRedisplay();

	//screenshotCondition.notify_all();
}

void ReshapeFunc(int width, int height) {
	resizeGL(width, height);
}

static void InitializeGlutCallbacks()
{
	glutWarpPointer(500, 500);

    glutDisplayFunc(RenderSceneCB);
	glutKeyboardFunc (KeyboardDown);
	glutKeyboardUpFunc (KeyboardUp);
	glutSpecialFunc(processSpecialKeyDown);
	glutSpecialUpFunc(processSpecialKeyUp);
	glutPassiveMotionFunc (MouseMoved);
	glutReshapeFunc(ReshapeFunc);
}

static HWND hwnd;
static DWORD WINAPI mainThreadId;
static bool captureInput = true;
static void globalHotkeyListener() {
	//AttachThreadInput(mainThreadId,GetCurrentThreadId(),true);

	RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'g');
	RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'G');
	//RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'f');
	//RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'F');
	RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, VK_F2);

	MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) && captureInput)
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



static int riftX = 0;
static int riftY = 0;
static int riftResolutionX = 0;
static int riftResolutionY = 0;
BOOL CALLBACK MonitorEnumProc(
  _In_  HMONITOR hMonitor,
  _In_  HDC hdcMonitor,
  _In_  LPRECT lprcMonitor,
  _In_  LPARAM dwData
) {
	MONITORINFOEX lpmi;
	MONITORINFO l;
	lpmi.cbSize = sizeof(MONITORINFOEX);
	char name[32];
	DISPLAY_DEVICE d;
	
	if(GetMonitorInfo(hMonitor, &lpmi)) {
		
		//strncpy(name, (const char*)lpmi.szDevice, 32);
		//if(strstr(name, "Rift") != NULL) {
		if( ((lpmi.rcMonitor.right-lpmi.rcMonitor.left) == riftResolutionX) &&
					((lpmi.rcMonitor.bottom-lpmi.rcMonitor.top) == riftResolutionY)) {
			riftX = lpmi.rcMonitor.left;
			riftY = lpmi.rcMonitor.top;
		}
	}
	return true;
}

static void getRiftDisplay() {
	if(InfoLoaded) {
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
		//char name[32];
		//DISPLAY_DEVICE d;
		//d.cb = sizeof(DISPLAY_DEVICE);
		//int i = 0;
		//while(EnumDisplayDevices(NULL, i++, &d, NULL)) {
		//	int i2 = 0;
		//	d.cb = sizeof(DISPLAY_DEVICE);
		//	DISPLAY_DEVICE ddMon;
		//	ZeroMemory(&ddMon, sizeof(ddMon));
		//	ddMon.cb = sizeof(ddMon);
		//	DWORD devMon = 0;
		//	while(EnumDisplayDevices(d.DeviceName, i2++, &ddMon, NULL)) {
		//		MONITORINFOEX mInfo;
		//		mInfo.cbSize = sizeof(MONITORINFOEX);
		//		GetMonitorInfo(ddMon.,&mInfo);
		//		if( ((mInfo.rcMonitor.right-mInfo.rcMonitor.left) == riftResolutionX) &&
		//			((mInfo.rcMonitor.top-mInfo.rcMonitor.bottom) == riftResolutionX)) {
		//		}

		//		d.cb = sizeof(DISPLAY_DEVICE);
		//		if(strstr((const char*)ddMon.DeviceString, "Rift") != NULL) {
		//			return;
		//			//riftX = lpmi.rcMonitor.left;
		//			//riftY = lpmi.rcMonitor.top;
		//		}
		//		devMon++;

		//		ZeroMemory(&ddMon, sizeof(ddMon));
		//		ddMon.cb = sizeof(ddMon);
		//	}
		//	d.cb = sizeof(DISPLAY_DEVICE);
		//}
	}
}
static void initRift() {
	OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));

	pManager = *OVR::DeviceManager::Create();

	//pManager->SetMessageHandler(this);

	pHMD = *pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();

	if (pHMD)
    {
		pSensor = *pHMD->GetSensor();

        InfoLoaded = pHMD->GetDeviceInfo(&Info);

		strncpy(Info.DisplayDeviceName, RiftMonitorName, 32);

		RiftDisplayId = Info.DisplayId;

		EyeDistance = Info.InterpupillaryDistance;
		DistortionK[0] = Info.DistortionK[0];
		DistortionK[1] = Info.DistortionK[1];
		DistortionK[2] = Info.DistortionK[2];
		DistortionK[3] = Info.DistortionK[3];
	}
	else
	{
		pSensor = *pManager->EnumerateDevices<OVR::SensorDevice>().CreateDevice();
	}

	if (pSensor)
	{
	   FusionResult.AttachToSensor(pSensor);

	   if(InfoLoaded) {
		   riftConnected = true;

		   riftX = Info.DesktopX;
		   riftY = Info.DesktopY;

		   riftResolutionX = Info.HResolution;
		   riftResolutionY = Info.VResolution;
	   }
	}

	getRiftDisplay();
}
static void cleanUpRift() {
	pSensor.Clear();
	pManager.Clear();

	OVR::System::Destroy();
}

// Get the horizontal and vertical screen sizes in pixel
static void GetDesktopResolution(int& horizontal, int& vertical)
{
   RECT desktop;
   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   // The top left corner will have coordinates (0,0)
   // and the bottom right corner will have coordinates
   // (horizontal, vertical)
   horizontal = desktop.right-desktop.left;
   vertical = desktop.bottom-desktop.top;
}

// add if you will call: dwmapi.lib
//BOOL IsAeroActive()
//{
//    // Check if Aero is enabled;
//	BOOL enabled = FALSE;
//    if (DwmIsCompositionEnabled(&enabled) == S_OK)
//    {
//        return enabled;
//    }
//    else
//    {
//        return false;
//    }
//}

static void exiting() {
	captureDesktop = false;
	captureInput = false;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	atexit(exiting);
	#ifdef _DEBUG
	RedirectIOToConsole();
	#endif

#if _USE_SIXENSE
	myInitSixense();
#endif

	int mainScreenHorizontal = 0;
	int mainScreenVertical = 0;
	GetDesktopResolution(mainScreenHorizontal, mainScreenVertical);

	initRift();
	FusionResult.Reset();

	width = 1280;
	height = 800;
	windowWidth = 1280;
	windowHeight = 800;
	if(riftConnected) {
        width = riftResolutionX;
        height = riftResolutionY;
        
        windowWidth = width;
        windowHeight = height;
	}
	physicalWidth = mainScreenHorizontal;//1280;//1920;//(false) ? 1920 : width;
	physicalHeight = mainScreenVertical;//800;//1080;//(false) ? 1080 : height;
	textureWidth = width*1.4;
	textureHeight = width*1.4;

	controlDesktop = 0;
	modifiedDesktop = true;

	size_t len = 0;
	wchar_t cwd[1024];
	GetCurrentDirectory(1023, cwd);
	wcstombs_s(&len, mResourcePath, cwd, 1023);

	int argc = 0;
	char **argv = 0;
    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_FULL_SCREEN);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);
	glutInitWindowPosition(riftX, riftY);
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

	videoPlayerContext = wglCreateContext(hdc);
	wglShareLists(mainContext, videoPlayerContext);

	mainThreadId = GetCurrentThreadId();

	std::thread screenshotThread(loopScreenshot);
	std::thread hotkeyThread(globalHotkeyListener);

	// add if you will call: dwmapi.lib
	//if(IsAeroActive()) 
	//{
		//SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		//SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

		//	HDC hdcBuffer = CreateCompatibleDC(hdc);
		//	POINT ptZero = {0, 0};
		//POINT ptDrawPos = {0, 0};
		//RECT rctCyauWnd;
		//GetWindowRect(hwnd, &rctCyauWnd);
		//SIZE szCyauWnd={rctCyauWnd.right - rctCyauWnd.left, rctCyauWnd.bottom - rctCyauWnd.top};
		//BLENDFUNCTION blendPixelFunction = { AC_SRC_OVER, 0, 100, AC_SRC_ALPHA};
		//	UpdateLayeredWindow(hwnd,
		//    hdc, &ptZero,
		//    &szCyauWnd,
		//    hdcBuffer, &ptZero,
		//    0, //RGB(255, 255, 255),
		//    &blendPixelFunction,
		//    ULW_ALPHA);
	//}

    glutMainLoop();

	cleanUpRift();

	captureDesktop = false;
	captureInput = false;
	std::terminate();

	screenshotThread.join();
	hotkeyThread.join();

	return 0;
}
