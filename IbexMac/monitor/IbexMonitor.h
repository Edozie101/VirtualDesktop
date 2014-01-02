/*
 * IbexMonitor.h
 *
 *  Created on: Jan 1, 2014
 *      Author: Hesham Wahba
 */

#ifndef IBEX_MONITOR_H_
#define IBEX_MONITOR_H_

#ifdef WIN32
#include "../stdafx.h"
#endif

#include <mutex>
extern std::condition_variable screenshotCondition;

namespace Ibex {

class IbexMonitor
{
public:
#ifdef WIN32
	IbexMonitor(const HDC &hdc, const HGLRC &mainContext, const HWND &captureDesktopHWND);
#else
#ifdef __APPLE__
    IbexMonitor();
#endif
#endif
	~IbexMonitor();

#ifdef WIN32
	void mergeMouseCursor(HDC hdcMemDC);
	int CaptureAnImage(HWND hWnd);
	void getScreenshot();
	void loopScreenshot();


private:
	HWND captureDesktopHWND;
	HGLRC loaderContext;
	HDC hdc;
	bool captureDesktop;

	CURSORINFO cursorinfo;
	ICONINFO ii;
	HCURSOR prevCursor;

	char *lpbitmap;
	bool used;

	double timeprev;
	double timebase;
	double frame;
	char fpsString[64];
#endif
	std::mutex screenshotMutex;
	std::unique_lock<std::mutex> *screenshotLock;
};

}

#endif
