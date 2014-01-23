#include "IbexMonitor.h"

#include "../opengl_helpers.h"

#include <condition_variable>
#include <algorithm>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>

#include <string.h>
#include <stdio.h>
#include <wchar.h>

#include "../ibex.h"
#include "../simpleworld_plugin/SimpleWorldRendererPlugin.h"

#include "../simpleworld_plugin/Rectangle.h"

std::condition_variable screenshotCondition;

#ifdef __APPLE__
::Ibex::IbexMonitor::IbexMonitor() :
     screenshotMutex()
    ,desktopTextures()
	,screenshotLock(0)
	,initialOffsetX(0)
	,initialOffsetY(0)
{
}
#endif

#ifdef WIN32
Ibex::IbexMonitor::IbexMonitor(const HDC &hdc, const HGLRC &mainContext, const std::vector<RECT> &desktopRects) :
screenshotMutex()
	,screenshotLock(0)
	,desktopRects(desktopRects)
	,hdc(hdc)
	,loaderContext(wglCreateContext(hdc))
	,captureDesktop(true)
	,prevCursor(0)
	,timeprev(glfwGetTime())
	,timebase(glfwGetTime())
	,frame(0)
	,fpsString()
	,initialOffsetX(0)
	,initialOffsetY(0)
{
	//bool result = wglShareLists(loaderContext, mainContext); // Order matters
	bool result = wglShareLists(mainContext, loaderContext); // Order matters
	std::cerr << "Initialized IbexMonitor shareGLLists: " << result << std::endl;
    
    desktopScaleFactors.clear();
    for(int i = 0; i < desktopRects.size(); ++i) {
        desktopScaleFactors.push_back(1.0f);
    }

	memset(&cursorinfo, 0, sizeof(CURSORINFO));
	memset(&ii, 0, sizeof(ICONINFO));
    
    initializeBounds();
}
#endif

Ibex::IbexMonitor::~IbexMonitor(void)
{
}

void Ibex::IbexMonitor::initializeBounds() {
    for(const RECT &r : desktopRects) {
		if(r.top == 0 && r.left == 0) {
			initialOffsetX = float(r.right-r.left)/2.0;
			initialOffsetY = float(r.bottom-r.top)/2.0;
		}
	}
    
    float minX = 0, maxX = 0, minY = 0, maxY = 0;
	for(int i = 0; i < desktopRects.size(); ++i) {
		minX = std::min(minX, (float)desktopRects[i].left);
		maxX = std::max(maxX, (float)desktopRects[i].right);
		minY = std::min(minY, (float)desktopRects[i].top);
		maxY = std::max(maxY, (float)desktopRects[i].bottom);
	}
    
	monitorBounds = glm::vec4(minX-initialOffsetX, -minY+initialOffsetY, maxX-initialOffsetX, -maxY+initialOffsetY)/100.0f;
}
void Ibex::IbexMonitor::initializeTextures() {
	for(int i = 0; i < desktopRects.size(); ++i) {
		const int w = (desktopRects[i].right-desktopRects[i].left)*desktopScaleFactors[i];
		const int h = (desktopRects[i].bottom-desktopRects[i].top)*desktopScaleFactors[i];
		
		usedTexture.push_back(false);
#ifdef WIN32
		bitmapCache.push_back(0);
#endif
		heightRatios.push_back(float(h)/float(w));
		desktopTextures.push_back(0);
		glGenTextures(1, &desktopTextures[i]);

		glBindTexture(GL_TEXTURE_2D, desktopTextures[i]);
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (!checkForErrors()) {
			std::cerr << "Stage 0c - Problem generating desktop FBO" << std::endl;
			exit(EXIT_FAILURE);
		}
#ifdef __APPLE__
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 0);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
#endif
		glBindTexture(GL_TEXTURE_2D, 0);
		checkForErrors();
	}
    
    initializeBounds();
}

void Ibex::IbexMonitor::renderIbexDisplayFlat(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP)
{
    static ::Ibex::Rectangle rectangle(0.5f);
	for(int i = 0; i < desktopTextures.size(); ++i) {
        const RECT &r = desktopRects[i];
        const float w = r.right-r.left;
        const float h = r.bottom-r.top;
        const float x = r.left+w/2.0f;
        const float y = -r.top-h/2.0f;
        
        const glm::mat4 translate = glm::translate((x-initialOffsetX)/100.0f, (y+initialOffsetY)/100.0f, 0.0f);
        const glm::mat4 scale = glm::scale(w/100.0f, h/100.0f*w/h, 1.0f);
        const glm::mat4 MVP2(MVP*translate*scale);
        const glm::mat4 M2(M*translate*scale);
        
        rectangle.render(MVP2, V, M2, shadowPass, depthMVP, desktopTextures[i], false, 0);
	}
}

glm::vec4 Ibex::IbexMonitor::getBounds() {
	return monitorBounds;
}

#ifdef WIN32
inline void Ibex::IbexMonitor::mergeMouseCursor(HDC hdcMemDC)
{
	return;
	cursorinfo.cbSize = sizeof(cursorinfo);

	const bool hasCursorInfo = GetCursorInfo(&cursorinfo);
	if(hasCursorInfo && cursorinfo.hCursor != prevCursor)  {
		prevCursor = cursorinfo.hCursor;
		GetIconInfo(cursorinfo.hCursor, &ii);
	}

	if(hasCursorInfo) 
	{
		DrawIconEx(hdcMemDC, cursorinfo.ptScreenPos.x-ii.xHotspot-physicalOffsetX,  cursorinfo.ptScreenPos.y-ii.yHotspot-physicalOffsetY, cursorinfo.hCursor, 0, 0, 0, NULL, DI_NORMAL);
	}
}

inline int Ibex::IbexMonitor::CaptureAnImage(const HWND &hWnd, const int &desktopNum)
{
	const RECT &rcClient = desktopRects[desktopNum];
	const GLuint &desktopTexture =  desktopTextures[desktopNum];

	//HDC hdcScreen;
	HDC hdcWindow;
	HDC hdcMemDC = NULL;
	HBITMAP hbmScreen = NULL;
	BITMAP bmpScreen;

	// Retrieve the handle to a display device context for the client 
	// area of the window. 
	//hdcScreen = GetDC(NULL);
	hdcWindow = GetDC(hWnd);

	// Create a compatible DC which is used in a BitBlt from the window DC
	hdcMemDC = CreateCompatibleDC(hdcWindow); 

	if(!hdcMemDC)
	{
		MessageBox(hWnd, L"CreateCompatibleDC has failed",L"Failed", MB_OK);
		goto done;
	}

	// Get the client area for size calculation
	//RECT rcClient;
	//GetClientRect(hWnd, &rcClient);

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

	// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
	// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
	// have greater overhead than HeapAlloc.
	//HANDLE hDIB = GlobalAlloc(GHND,dwBmpSize); 
	//char *lpbitmap = (char *)GlobalLock(hDIB); 
	char *lpbitmap = bitmapCache[desktopNum];
	if(lpbitmap == 0) {
		DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 23) / 24) * 3 * bmpScreen.bmHeight;

		lpbitmap = new char[dwBmpSize];
		bitmapCache[desktopNum] = lpbitmap;
	}
	screenshotCondition.wait(*screenshotLock);

	// Bit block transfer into our compatible memory DC.
	if(!BitBlt(hdcMemDC, 
		0,0, 
		rcClient.right-rcClient.left, rcClient.bottom-rcClient.top, 
		hdcWindow, 
		rcClient.left, rcClient.top,//0,0,
		SRCCOPY | CAPTUREBLT))
	{
		MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
		goto done;
	}
	mergeMouseCursor(hdcMemDC);


	// Gets the "bits" from the bitmap and copies them into a buffer 
	// which is pointed to by lpbitmap.
	int result = GetDIBits(hdcWindow, hbmScreen, 0,
		(UINT)bmpScreen.bmHeight,
		lpbitmap,
		(BITMAPINFO *)&bi, DIB_RGB_COLORS);

	if(desktopTexture) {
		glBindTexture(GL_TEXTURE_2D, desktopTexture);
		if(usedTexture[desktopNum]) {
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bmpScreen.bmWidth, bmpScreen.bmHeight, GL_BGR, GL_UNSIGNED_BYTE, lpbitmap);
		} else {
			usedTexture[desktopNum] = true;
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
	//ReleaseDC(NULL,hdcScreen);
	ReleaseDC(hWnd,hdcWindow);

	return 0;
}

void Ibex::IbexMonitor::getScreenshot() {
	static HWND hwnd = GetDesktopWindow();
	for(int i = 0; i < desktopRects.size(); ++i) {
		CaptureAnImage(hwnd, i);
	}
}

void Ibex::IbexMonitor::loopScreenshot() {
	screenshotLock = new std::unique_lock<std::mutex>(screenshotMutex);

	wglMakeCurrent(hdc, loaderContext);
	initializeTextures();

	while(captureDesktop) {
		getScreenshot();

#ifdef _DEBUG
		const double time = glfwGetTime();
		timeprev = time;
		++frame;
		if (time - timebase >= 5.0) {
			sprintf(fpsString,"FPS:%4.2f", frame*5.0/(time-timebase));
			std::cerr << "Capture " << fpsString << std::endl;
			timebase = time;
			frame = 0;
		}
#endif
	}
	delete screenshotLock;
}
#endif
