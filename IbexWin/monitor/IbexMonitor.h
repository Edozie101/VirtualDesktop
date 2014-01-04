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

#include <vector>
#include <mutex>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "../GLSLShaderProgram.h"

extern std::condition_variable screenshotCondition;

#ifdef __APPLE__
typedef struct __RECT {
    const int left,right,top,bottom;
    __RECT(const int &l, const int &t, const int &r, const int &b)
        : left(l),right(r),top(t),bottom(b) {}
} RECT;
#endif

namespace Ibex {

class IbexMonitor
{
public:
#ifdef WIN32
	IbexMonitor(const HDC &hdc, const HGLRC &mainContext, const std::vector<RECT> &desktopRects);
#else
#ifdef __APPLE__
    IbexMonitor();
#endif
#endif
	~IbexMonitor();

	void initializeTextures();
	void renderIbexDisplayFlat(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);

	glm::vec4 getBounds();
    void initializeBounds();
    
#ifdef WIN32
	void mergeMouseCursor(HDC hdcMemDC);
	int CaptureAnImage(const HWND &hWnd, const int &desktopNum);
	void getScreenshot();
	void loopScreenshot();
    
private:
    std::vector<char*> bitmapCache;
	HGLRC loaderContext;
	HDC hdc;
	bool captureDesktop;

	CURSORINFO cursorinfo;
	ICONINFO ii;
	HCURSOR prevCursor;
#endif
    
public:
    std::vector<bool> usedTexture;
	std::vector<RECT> desktopRects;
	std::vector<GLuint> desktopTextures;
	std::vector<float> heightRatios;
    
    double timeprev;
	double timebase;
	double frame;
	char fpsString[64];
    
	std::mutex screenshotMutex;
	std::unique_lock<std::mutex> *screenshotLock;

	float initialOffsetX;
	float initialOffsetY;
	glm::vec4 monitorBounds;
};

}

extern Ibex::IbexMonitor *ibexMonitor;

#endif
