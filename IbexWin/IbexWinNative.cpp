// IbexWinNative.cpp : Defines the entry point for the application.
//

#ifdef WIN32
#include "video/VLCVideoPlayer.h"
#endif

#include "stdafx.h"

#include "IbexWinNative.h"
#include "opengl_helpers.h"
#include "ibex_win_utils.h"

#if _USE_SIXENSE
#include "sixense/sixense_controller.h"
#endif

// add if you will call: dwmapi.lib
// #include <Dwmapi.h>

#include <condition_variable>
#include <algorithm>
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
#include <wchar.h>

typedef unsigned long Window;
typedef unsigned long GLXContext;

#include "math_3d.h"

#include "ibex.h"
#include "RendererPlugin.h"

#include "distortions.h"

#include "monitor/IbexMonitor.h"
#include "windows/ApplicationLauncher.h"

bool modifiedDesktop(false);
GLuint VBO(0);

Ibex::Ibex *ibex = 0;
GLFWwindow* glfwWindow;
Ibex::IbexMonitor *ibexMonitor = 0;

static inline void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {//int key, int action) {
	int down = action != GLFW_RELEASE;
	int processed = 0;
	if(showDialog) {
		processed = ibex->renderer->window.processKey(key, down);
	}
	if(showApplicationLauncher) {
		processed = ibex->renderer->applicationLauncher->processKey(key, down);
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
		case GLFW_KEY_SEMICOLON:
			if(!down) {
				useLightPerspective = !useLightPerspective;
			}
			break;
		case 'l':
		case 'L':
			if(!down) {
				lockHeadTracking = !lockHeadTracking;
				settingChangedMessage = "Rift Head Tracking: "+std::string((lockHeadTracking)?"ON":"OFF");
				settingChanged = true;
			}
            break;
		case 'u':
		case 'U':
			if(!down) {
				walkLockedToView = !walkLockedToView;
				settingChangedMessage = "Walking Locked to View: "+std::string((walkLockedToView)?"ON":"OFF");
				settingChanged = true;
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
		case 'o':
		case 'O':
			if(!down) {
				showApplicationLauncher = !showApplicationLauncher;
			}
			break;
        case 'h':
        case 'H':
		case GLFW_KEY_SLASH:
			if(!down) {
				showDialog = ibex->renderer->window.toggleShowDialog();
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
				settingChangedMessage = "Show Ground: "+std::string((showGround)?"ON":"OFF");
                settingChanged = true;
			}
			break;
		case 'r':
		case 'R':
			if(!down) {
				resetPosition = 1;
				settingChangedMessage = "Reset Position";
                settingChanged = true;
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

HDC hdc = 0;
bool captureDesktop = true;
HGLRC videoPlayerContext = NULL;
Ibex::VLCVideoPlayer *_ibexVideoPlayer = NULL;
void makeCurrentGL() {
	bool success = wglMakeCurrent(hdc, videoPlayerContext);
	if(!success) {
		std::cerr << "Video playing wglMakeCurrent: " << success << std::endl;
	}
}
static void playVideo() {
	//bool success = wglMakeCurrent(hdc, videoPlayerContext);
	//std::cerr << "Video playing wglMakeCurrent: " << success << std::endl;

	if(_ibexVideoPlayer != NULL) {
		delete _ibexVideoPlayer;
		_ibexVideoPlayer = NULL;
	}
	_ibexVideoPlayer = new Ibex::VLCVideoPlayer();
	isSBSVideo = ibex->renderer->window.getSBSVideo();
	_ibexVideoPlayer->playVideo(ibex->renderer->window.getSelectedVideoPath().c_str(),ibex->renderer->window.getIsStereoVideo(), 0, 0, &makeCurrentGL);
}
static void playCamera() {
	bool success = wglMakeCurrent(hdc, videoPlayerContext);
	std::cerr << "Video playing wglMakeCurrent: " << success << std::endl;

	if(_ibexVideoPlayer == NULL) {
		_ibexVideoPlayer = new Ibex::VLCVideoPlayer();
	} else {
		_ibexVideoPlayer->stopCapturing();
	}
	isSBSVideo = 0;
	_ibexVideoPlayer->openCamera(ibex->renderer->window.getIsStereoVideo(), ibex->renderer->window.getSelectedCameraID());
}

static void RenderSceneCB()
{
	screenshotCondition.notify_all();
	static double timeprev = glfwGetTime();
	double time = glfwGetTime();
	double timeDiff = (time - timeprev);
	timeprev = time;

	static double timebase = glfwGetTime();
	static double frame = 0;
	++frame;
	//static char fpsString[64];
	if (time - timebase >= 5.0) {
		sprintf(fpsString,"FPS:%4.2f", frame*5.0/(time-timebase));
		#ifdef _DEBUG
		std::cerr << fpsString << std::endl;
		#endif
		timebase = time;
		frame = 0;
	}

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
		cursorPosX = p.x;//-physicalOffsetX;
		cursorPosY = p.y;///*physicalHeight*/-p.y-physicalOffsetY;
	}

	if(ibex != NULL && (ibex->renderer->window.getSelectedVideo() || ibex->renderer->window.getSelectedCamera())) {
		if(ibex->renderer->window.getSelectedVideo()) {
			static std::thread *videoThread = 0;
			if(videoThread) {
				delete videoThread;
				videoThread = 0;
			}
			videoThread = new std::thread(playVideo);
			videoThread->detach();
		} else {
			std::thread videoThread(playCamera);
			videoThread.detach();
		}
		ibex->renderer->window.setSelectedVideo(false);
		ibex->renderer->window.setSelectedCamera(false);
	}
	static const GLuint staticTexture = loadTexture("\\resources\\static.jpg");
	if(_ibexVideoPlayer != NULL) {
		videoIsNoise = false;
		videoTexture[0] = _ibexVideoPlayer->videoTexture[0];
		videoTexture[1] = _ibexVideoPlayer->videoTexture[1];
		videoWidth = _ibexVideoPlayer->width;
		videoHeight = _ibexVideoPlayer->height;
	} else {
		videoIsNoise = true;
		videoTexture[0] = videoTexture[1] = staticTexture;
		videoWidth = 1280;
		videoHeight = 720;
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


	RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_SHIFT /* | MOD_NOREPEAT */, 'g'); // MOD_NOREPEAT not available on XP
	RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT /*| MOD_NOREPEAT*/, 'G'); // MOD_NOREPEAT not available on XP
	//RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT /*| MOD_NOREPEAT*/, 'f'); // MOD_NOREPEAT not available on XP
	//RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT /*| MOD_NOREPEAT*/, 'F'); // MOD_NOREPEAT not available on XP
	RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_SHIFT /*| MOD_NOREPEAT*/, VK_F2); // MOD_NOREPEAT not available on XP

	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0) && captureInput)
	{
		//TranslateMessage(&msg);
		//DispatchMessage(&msg);
		if(msg.message == WM_HOTKEY) {
			controlDesktop = !controlDesktop;

			settingChangedMessage = "Control Desktop: "+std::string((controlDesktop)?"ON":"OFF");
            settingChanged = true;

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
	textureWidth = width*1.4;
	textureHeight = width*1.4;

	controlDesktop = 0;
	modifiedDesktop = true;

	size_t len = 0;
	wchar_t cwd[1024];
	GetCurrentDirectory(1023, cwd);
	wcstombs_s(&len, mResourcePath, cwd, 1023);

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);


	int monitorCount = 0;
	GLFWmonitor* monitor = NULL;//glfwGetPrimaryMonitor();
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

	std::vector<RECT> monitorRECTs;
	std::vector<HWND> monitorHWNDs;
	std::vector<Ibex::IbexMonitor*> ibexMonitors;
	bool isRiftPrimaryMonitor = false;
	int searchRiftResolutionX = GetPrivateProfileInt(L"rift", L"resolutionX", 1280, L".\\ibex.ini");
	int searchRiftResolutionY = GetPrivateProfileInt(L"rift", L"resolutionY", 800, L".\\ibex.ini");
	for(int i = 0; i < monitorCount; ++i) {
		std::cerr << "Monitor Id[" << i << "]: " << (monitors[i]) << std::endl;
		
		std::string monitorName(glfwGetMonitorName(monitors[i]));
		std::transform(monitorName.begin(), monitorName.end(), monitorName.begin(), ::tolower);
		std::cerr << "Monitor Name[" << i << "]: " << monitorName << std::endl;

		const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
		if((monitorName.find("rift dk") != std::string::npos) || (mode->width == searchRiftResolutionX && mode->height == searchRiftResolutionY)) {
			monitor = monitors[i];
			isRiftPrimaryMonitor = (monitor == primaryMonitor);
			if(monitorName.find("rift dk") != std::string::npos) {
				// found specifically named Rift display so can exit loop early
				break;
			}
		}
	}

	// if Rift is primaryMonitor set captureDesktopHWND to second desktop so it can be captured
	HWND captureDesktopHWND = 0;
	int mainScreenHorizontal = 0;
	int mainScreenVertical = 0;
	GetDesktopResolution(mainScreenHorizontal, mainScreenVertical);
	//if(isRiftPrimaryMonitor) {
	//	for(int i = 0; i < monitorCount; ++i) {
	//		std::cerr << "Monitor Id[" << i << "]: " << (monitors[i]) << std::endl;

	//		std::string monitorName(glfwGetMonitorName(monitors[i]));
	//		std::transform(monitorName.begin(), monitorName.end(), monitorName.begin(), ::tolower);
	//		std::cerr << "Monitor Name[" << i << "]: " << monitorName << std::endl;

	//		const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
	//		if(monitors[i] != monitor) {
	//			int xpos=0,ypos=0;
	//			glfwGetMonitorPos(monitors[i], &xpos, &ypos);

	//			physicalOffsetX = xpos;
	//			physicalOffsetY = ypos;
	//			physicalWidth = mode->width;
	//			physicalHeight = mode->height;

	//			POINT p;
	//			p.x = xpos;
	//			p.y = ypos;
	//			captureDesktopHWND = WindowFromPoint(p);

	//			//break;
	//			monitorHWNDs.push_back(captureDesktopHWND);
	//		}
	//	}
	//}
	for(int i = 0; i < monitorCount; ++i) {
		std::cerr << "Monitor Id[" << i << "]: " << (monitors[i]) << std::endl;

		std::string monitorName(glfwGetMonitorName(monitors[i]));
		std::transform(monitorName.begin(), monitorName.end(), monitorName.begin(), ::tolower);
		std::cerr << "Monitor Name[" << i << "]: " << monitorName << std::endl;

		const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
		//if(monitors[i] != monitor || isRiftPrimaryMonitor) {
			int xpos=0,ypos=0;
			glfwGetMonitorPos(monitors[i], &xpos, &ypos);

			physicalOffsetX = xpos;
			physicalOffsetY = ypos;
			//physicalWidth = mode->width;
			//physicalHeight = mode->height;

			POINT p;
			p.x = xpos;
			p.y = ypos;
			HWND hwnd = WindowFromPoint(p);

			RECT rcClient;
			SetRect(&rcClient, xpos, ypos, xpos+mode->width, ypos+mode->height);
			monitorRECTs.push_back(rcClient);
			monitorHWNDs.push_back(hwnd);
		//}
	}

	//physicalWidth = mainScreenHorizontal;//1280;//1920;//(false) ? 1920 : width;
	//physicalHeight = mainScreenVertical;//800;//1080;//(false) ? 1080 : height;

	int xpos = 0;
	int ypos = 0;
	if(monitor == NULL) {
		width = 1280;
		height = 800;
	} else {
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		width = mode->width;
		height = mode->height;
		glfwGetMonitorPos(monitor, &xpos, &ypos);

		windowWidth = width;
		windowHeight = height;
	}

	initRift();
	_TCHAR renderScaleString[1000];
	_TCHAR *stopString;
	GetPrivateProfileString(L"rift", L"renderScale", L"0", renderScaleString, sizeof(renderScaleString), L".\\ibex.ini");
	double overrideRenderScale = wcstod(renderScaleString, &stopString);
	if(overrideRenderScale >= 1.0) {
		setRenderScale(overrideRenderScale);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	
	if(riftConnected) {
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	}
	//glfwWindow = glfwCreateWindow(width, height, "Ibex", (riftConnected) ? monitor : NULL, NULL);
	glfwWindow = glfwCreateWindow(width, height, "Ibex", NULL, NULL);
	if (!glfwWindow)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	if(riftConnected) {
		glfwSetWindowPos(glfwWindow, xpos, ypos);
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

	// for some reason must do this before sharing lists of mainContext with videoPlayerContext
	// passing in captureDesktopHWND but in future can set to specific desktop to capture when multiple monitors
	//Ibex::IbexMonitor m(hdc, mainContext, captureDesktopHWND);
	//ibexMonitors.push_back(new Ibex::IbexMonitor(hdc, mainContext, monitorRECTs));
	ibexMonitor = new Ibex::IbexMonitor(hdc, mainContext, monitorRECTs);

	videoPlayerContext = wglCreateContext(hdc);
	wglShareLists(mainContext, videoPlayerContext);

	mainThreadId = GetCurrentThreadId();

	//std::thread screenshotThread(&Ibex::IbexMonitor::loopScreenshot, &m);
	std::thread screenshotThread(&Ibex::IbexMonitor::loopScreenshot, ibexMonitor);
	//int i = 0;
	//for(Ibex::IbexMonitor *m : ibexMonitors) {
	//	if(i++ == 0) continue;
	//	std::thread *screenshotThread = new std::thread(&Ibex::IbexMonitor::loopScreenshot, m);
	//	//break;
	//}
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

	//screenshotThread.join();
	hotkeyThread.join();

	return 0;
}
