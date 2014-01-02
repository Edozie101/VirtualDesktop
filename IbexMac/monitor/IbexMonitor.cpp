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

std::condition_variable screenshotCondition;

#ifdef __APPLE__
::Ibex::IbexMonitor::IbexMonitor() :
screenshotMutex()
,screenshotLock(0)
{
}
#endif
    
#ifdef WIN32
::Ibex::IbexMonitor::IbexMonitor(const HDC &hdc, const HGLRC &mainContext, const HWND &captureDesktopHWND) :
	screenshotMutex()
	,screenshotLock(0)
	,captureDesktopHWND(captureDesktopHWND)
	,hdc(hdc)
	,loaderContext(wglCreateContext(hdc))
	,captureDesktop(true)
	,prevCursor(0)
	,lpbitmap(0)
	,used(false)
	,timeprev(glfwGetTime())
	,timebase(glfwGetTime())
	,frame(0)
	,fpsString()
{
	bool result = wglShareLists(loaderContext, mainContext); // Order matters
	std::cerr << "Initialized IbexMonitor shareGLLists: " << result << std::endl;

	memset(&cursorinfo, 0, sizeof(CURSORINFO));
	memset(&ii, 0, sizeof(ICONINFO));
}
#endif

::Ibex::IbexMonitor::~IbexMonitor(void)
{
}

#ifdef WIN32
inline void ::Ibex::IbexMonitor::mergeMouseCursor(HDC hdcMemDC)
{
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

inline int ::Ibex::IbexMonitor::CaptureAnImage(HWND hWnd)
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

	// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
	// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
	// have greater overhead than HeapAlloc.
	//HANDLE hDIB = GlobalAlloc(GHND,dwBmpSize); 
	//char *lpbitmap = (char *)GlobalLock(hDIB);    
	if(lpbitmap == 0) {
		DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 23) / 24) * 3 * bmpScreen.bmHeight;

		lpbitmap = new char[dwBmpSize];
	}
	screenshotCondition.wait(*screenshotLock);

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

void ::Ibex::IbexMonitor::getScreenshot() {
	HWND hwnd = (captureDesktopHWND) ? captureDesktopHWND : GetDesktopWindow();
	CaptureAnImage(hwnd);
}

void ::Ibex::IbexMonitor::loopScreenshot() {
	screenshotLock = new std::unique_lock<std::mutex>(screenshotMutex);

	wglMakeCurrent(hdc, loaderContext);
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
