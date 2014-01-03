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

#ifdef WIN32
	void mergeMouseCursor(HDC hdcMemDC);
	int CaptureAnImage(const HWND &hWnd, const int &desktopNum);
	void getScreenshot();
	void loopScreenshot();

private:
	std::vector<bool> usedTexture;
	std::vector<char*> bitmapCache;
	std::vector<RECT> desktopRects;
	std::vector<GLuint> desktopTextures;
	std::vector<float> heightRatios;
	HGLRC loaderContext;
	HDC hdc;
	bool captureDesktop;

	CURSORINFO cursorinfo;
	ICONINFO ii;
	HCURSOR prevCursor;

	double timeprev;
	double timebase;
	double frame;
	char fpsString[64];
#endif
	std::mutex screenshotMutex;
	std::unique_lock<std::mutex> *screenshotLock;

	float initialOffsetX;
	float initialOffsetY;
	glm::vec4 monitorBounds;
};

}

extern Ibex::IbexMonitor *ibexMonitor;

#endif
