// IbexWinNative.cpp : Defines the entry point for the application.
//

#ifdef _WIN32
#include "video/VLCVideoPlayer.h"
#endif

#include "stdafx.h"

#include "IbexWinNative.h"
#include "opengl_helpers.h"

#if _USE_SIXENSE
#include "sixense/sixense_controller.h"
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

#include "oculus/Rift.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

#include <stdio.h>

typedef unsigned long Window;
typedef unsigned long GLXContext;

#include "math_3d.h"

#include "ibex.h"
#include "RendererPlugin.h"

#include "distortions.h"

bool modifiedDesktop(false);
GLuint VBO(0);
std::condition_variable screenshotCondition;

Ibex::Ibex *ibex = 0;
GLFWwindow* glfwWindow;

static inline void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {//int key, int action) {
	int down = action != GLFW_RELEASE;
	int processed = 0;
	if(showDialog) {
		processed = ibex->renderer->window.processKey(key, down);
	}
	if(!processed) {
		switch (key)
		{
		case GLFW_KEY_LEFT_SHIFT:
		case GLFW_KEY_RIGHT_SHIFT:
			running = down;
			break;
		case GLFW_KEY_SPACE:
			jump = down;
			break;
		case GLFW_KEY_UP:
			walkForward = 1 * down;
			break;
		case GLFW_KEY_DOWN:
			walkForward = -1 * down;
			break;
		case GLFW_KEY_RIGHT:
			strafeRight = 1 * down;
			break;
		case GLFW_KEY_LEFT:
			strafeRight = -1 * down;
			break;
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
		case 'l':
		case 'L':
			if(!down) {
				useLightPerspective = !useLightPerspective;
			}
			break;
		case GLFW_KEY_BACKSLASH:
			if(!down) {
				bringUpIbexDisplay = true;
			}
			break;
		case '=':
		case '+':
			if(!down) {
				IOD += 0.0005;
				lensParametersChanged = true;
			}
			break;
		case 'Q':
		case 'q':
			if(mods & GLFW_MOD_CONTROL) {
				exit(0);
				break;
			}

			if(!down) {
				displayShape = (displayShape == FlatDisplay) ? SphericalDisplay : FlatDisplay;
			}
			break;
        case 'h':
        case 'H':
		case GLFW_KEY_SLASH:
			if(!down) {
				showDialog = !showDialog;
                ibex->renderer->window.reset();
                ibex->renderer->window.showDialog(showDialog, (key == 'h' || key == 'H')?::Ibex::HelpWindow : ::Ibex::InfoWindow);
			}
			break;
		case '-':
		case '_':
			if(!down) {
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

static inline void cursor_callback(GLFWwindow *window, double x, double y)
{
	if(!controlDesktop) {
		relativeMouseX = x-500;
		relativeMouseY = y-500;
	}
	//std::cerr << relativeMouseX << ", " << relativeMouseY << std::endl;
}

static inline void mergeMouseCursor(HDC hdcMemDC)
{
	static CURSORINFO cursorinfo = { 0 };
	static ICONINFO ii = {0};
	static HCURSOR prevCursor = 0;
	cursorinfo.cbSize = sizeof(cursorinfo);

	const bool hasCursorInfo = GetCursorInfo(&cursorinfo);
	if(hasCursorInfo && cursorinfo.hCursor != prevCursor)  {
		prevCursor = cursorinfo.hCursor;
		GetIconInfo(cursorinfo.hCursor, &ii);
	}

	if(hasCursorInfo) 
	{
		DrawIconEx(hdcMemDC, cursorinfo.ptScreenPos.x-ii.xHotspot,  cursorinfo.ptScreenPos.y-ii.yHotspot, cursorinfo.hCursor, 0, 0, 0, NULL, DI_NORMAL);
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
	mergeMouseCursor(hdcMemDC);


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
			//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
		}
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

		//glBindTexture(GL_TEXTURE_2D, 0);
	}

	// free []lpbitmap;

	//Unlock and Free the DIB from the heap
	//GlobalUnlock(hDIB);    
	//GlobalFree(hDIB);

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
		static double timeprev = glfwGetTime();
		const double time = glfwGetTime();
		timeprev = time;

		static double timebase = glfwGetTime();
		static double frame = 0;
		++frame;
		static char fpsString[64];
		if (time - timebase >= 5.0) {
			sprintf(fpsString,"FPS:%4.2f", frame*5.0/(time-timebase));
			std::cerr << "Capture " << fpsString << std::endl;
			timebase = time;
			frame = 0;
		}
#endif
	}
}

HGLRC videoPlayerContext = NULL;
Ibex::VLCVideoPlayer *_ibexVideoPlayer = NULL;
void makeCurrentGL() {
	bool success = wglMakeCurrent(hdc, videoPlayerContext);
	std::cerr << "Video playing wglMakeCurrent: " << success << std::endl;
}
static void playVideo() {
	//bool success = wglMakeCurrent(hdc, videoPlayerContext);
	//std::cerr << "Video playing wglMakeCurrent: " << success << std::endl;

	_ibexVideoPlayer = new Ibex::VLCVideoPlayer();
	_ibexVideoPlayer->playVideo(ibex->renderer->window.getSelectedVideoPath().c_str(),ibex->renderer->window.getIsStereoVideo(), 0, 0, &makeCurrentGL);
	//_ibexVideoPlayer->openCamera(ibex->renderer->window.getIsStereoVideo(), -1);
}
static void playCamera() {
	bool success = wglMakeCurrent(hdc, videoPlayerContext);
	std::cerr << "Video playing wglMakeCurrent: " << success << std::endl;

	_ibexVideoPlayer = new Ibex::VLCVideoPlayer();
	_ibexVideoPlayer->openCamera(ibex->renderer->window.getIsStereoVideo(), ibex->renderer->window.getSelectedCameraID());
}

static void RenderSceneCB()
{
	screenshotCondition.notify_all();
	static double timeprev = glfwGetTime();
	double time = glfwGetTime();
	double timeDiff = (time - timeprev);
	timeprev = time;

#ifdef _DEBUG
	static double timebase = glfwGetTime();
	static double frame = 0;
	++frame;
	static char fpsString[64];
	if (time - timebase >= 5.0) {
		sprintf(fpsString,"FPS:%4.2f", frame*5.0/(time-timebase));
		std::cerr << fpsString << std::endl;
		timebase = time;
		frame = 0;
	}
#endif

	if(modifiedDesktop) {
		modifiedDesktop = false;
		if(controlDesktop) {
			glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		} else {
			glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
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

	//screenshotCondition.notify_all();
}

void window_size_callback(GLFWwindow* window, int width, int height) {
	resizeGL(width, height);
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

///////////////// compositing management (from comments on http://weblogs.asp.net/kennykerr/comments/429272.aspx) ///////////////////////////////
typedef HRESULT (CALLBACK * P_DwmIsCompositionEnabled)(BOOL *pfEnabled);
typedef HRESULT (CALLBACK * P_DwmEnableComposition)   (BOOL   fEnable);

struct DWMAPILib {
	HMODULE	 lib;
	P_DwmIsCompositionEnabled	fIsEnabled;
	P_DwmEnableComposition	 fEnable;

	DWMAPILib() : lib((HMODULE)0xFFFFFFFF), fIsEnabled(NULL), fEnable(NULL) {}
	~DWMAPILib() {
		if (lib != NULL && lib != (HMODULE)0xFFFFFFFF)
			::FreeLibrary(lib);
		lib = (HMODULE)0xFFFFFFFF;
	}

	BOOL Load() {
		if (lib == NULL) return FALSE;
		lib = ::LoadLibrary(L"dwmapi.dll");
		if (lib == NULL) return FALSE;
		fIsEnabled = (P_DwmIsCompositionEnabled)::GetProcAddress(lib, "DwmIsCompositionEnabled");
		fEnable    = (P_DwmEnableComposition)   ::GetProcAddress(lib, "DwmEnableComposition");
		return TRUE;
	}
	bool IsCompositionEnabled() {
		BOOL	enabled = FALSE;
		return (fIsEnabled != NULL && SUCCEEDED(fIsEnabled(&enabled)) && enabled);
	}
	HRESULT EnableComposition(BOOL enable) {
		if (!fIsEnabled) return 0x80070000 + ERROR_PROC_NOT_FOUND;  // @@@efh really should get from GetLastError / ERROR_MOD_NOT_FOUND if lib == NULL
		return fEnable(enable);
	}
};

static DWMAPILib	dwmapiLib;
static BOOL	 compositingWasEnabled = false;
bool isVistaOrLaterForDWMAPIDLL() {
   OSVERSIONINFO osVersionInfo;
   ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFO));
   osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(&osVersionInfo);
   return osVersionInfo.dwMajorVersion >= 6;
}
void disableCompositing()
{
	if(!isVistaOrLaterForDWMAPIDLL()) return;
	if (!dwmapiLib.Load()) return;
	if ((compositingWasEnabled = dwmapiLib.IsCompositionEnabled()) == true)
		dwmapiLib.EnableComposition(FALSE);	 // DWM_EC_DISABLECOMPOSITION
}

void restoreCompositing() {
	if(!isVistaOrLaterForDWMAPIDLL()) return;
	if (compositingWasEnabled && dwmapiLib.Load())
		dwmapiLib.EnableComposition(TRUE);	 // DWM_EC_ENABLECOMPOSITION
}
/////////////////



static void error_callback(int error, const char* description)
{
	std::cerr << "GLFW Error(" << error << "): " << description << std::endl;
}

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

	//int argc = 0;
	//char **argv = 0;
	//   glutInit(&argc, argv);
	//glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_FULL_SCREEN);
	//   glutInitWindowSize(width, height);
	//   glutInitWindowPosition(0, 0);
	//glutInitWindowPosition(riftX, riftY);
	//   glutCreateWindow("Ibex");

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);


	int monitorCount = 0;
	GLFWmonitor* monitor = NULL;//glfwGetPrimaryMonitor();
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
	for(int i = 0; i < monitorCount; ++i) {
		std::cerr << "Monitor Name[" << i << "]: " << (monitors[i]) << std::endl;
		const char *monitorName = glfwGetMonitorName(monitors[i]);
		const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
		if((strcmp(monitorName, /*RiftMonitorName*/"Rift DK") == 0)
			|| (mode->width == 1280 && mode->height == 800)) {
			monitor = monitors[i];
			break;
		}
	}
	if(monitor == NULL) {
		width = 1280;
		height = 800;
	} else {
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		width = mode->width;
		height = mode->height;
	}

	initRift();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindow = glfwCreateWindow(width, height, "Ibex", (riftConnected) ? monitor : NULL, NULL);
	if (!glfwWindow)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(glfwWindow);
	glfwSetWindowSizeCallback(glfwWindow, window_size_callback);
	glfwSetKeyCallback(glfwWindow, key_callback);
	glfwSetCursorPosCallback(glfwWindow, cursor_callback);

	// Must be done after GLFW is initialized!
	// Initialize GLEW
	glewExperimental=true; // Needed in core profile
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

	disableCompositing();
	// main loop
	while (!glfwWindowShouldClose(glfwWindow))
	{
		RenderSceneCB();
		glfwSwapBuffers(glfwWindow);
		glfwPollEvents();
		if(!controlDesktop) {
			glfwSetCursorPos(glfwWindow, 500,500);
		}
	}
	glfwDestroyWindow(glfwWindow);
	glfwTerminate();

	restoreCompositing();

	cleanUpRift();

	captureDesktop = false;
	captureInput = false;
	std::terminate();

	screenshotThread.join();
	hotkeyThread.join();

	return 0;
}
