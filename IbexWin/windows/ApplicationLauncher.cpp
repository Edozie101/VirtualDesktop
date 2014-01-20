//
//  ApplicationLauncher.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 1/10/14.
//  Copyright (c) 2014 Hesham Wahba. All rights reserved.
//

#include "ApplicationLauncher.h"

#ifdef WIN32
#include "../ibex_win_utils.h"
#include "../stdafx.h"
#endif
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include "../ibex_mac_utils.h"

void launchApplication(const std::string &applicationPath);
#endif

#include "../simpleworld_plugin/SimpleWorldRendererPlugin.h"

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#include "../filesystem/Filesystem.h"

Ibex::ApplicationLauncher::ApplicationLauncher() : ww(0),
	hh(0),
	appTexture(0),
	appSelectionTexture(0),
	selectedX(0),
	selectedY(0),
	newX(0),
	newY(0),
	first(true)
#ifdef WIN32
		,lpbitmap(0)
#endif
{
}

#ifdef WIN32
bool launchApplication( const char* path, HWND parent = NULL ) {
	HINSTANCE result = ShellExecuteA( parent, NULL, path, NULL, NULL, SW_SHOWNORMAL );

	if ((int)result == SE_ERR_ACCESSDENIED)
		result = ShellExecuteA( parent, "runas", path, NULL, NULL, SW_SHOWNORMAL );

	return ((int)result > 32);
}

void drawIconToDC(HICON hIcon, HDC inHDC,int posx, int posy) {
	int x = 96, y = 96;
	HDC hDC = GetDC(NULL);
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hMemBmp = CreateCompatibleBitmap(hDC, x, y);
	HBITMAP hResultBmp = NULL;
	HGDIOBJ hOrgBMP = SelectObject(hMemDC, hMemBmp);

	DrawIconEx(inHDC, posx, posy, hIcon, x, y, 0, NULL, DI_NORMAL);

	// need to flip icon b/c it is upside-down
	StretchBlt(
		inHDC,
		posx,
		posy,
		x,
		y,
		inHDC,
		posx,
		posy+y,
		x,
		-y,
		SRCCOPY
		);

	DeleteObject(hMemBmp);
	SelectObject(hMemDC, hOrgBMP);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hDC);
	DestroyIcon(hIcon);
	return;
}

GLuint Ibex::ApplicationLauncher::createApplicationLauncherBitmap(std::vector<std::string> &paths, size_t &width, size_t &height, int &selectedX, int &selectedY,std::map<std::pair<int,int>,std::string> &applicationList) {
	const int iconRes = 96;
	const int iconSpacing = 8;

	int appCount = 0;

	for(int p = 0; p < paths.size(); ++p) {
		const char *path_ = paths[p].c_str();
		std::vector<std::string> appDirectory = Filesystem::listDirectory(path_);
		for(int i = 0; i < appDirectory.size(); ++i) {
			if(appDirectory[i].find(".lnk") == std::string::npos && appDirectory[i].find(".exe") == std::string::npos) continue;
			++appCount;
		}
	}

	const int vert = 8;
	const int horiz = ceil(float(appCount)/float(vert));
	if(horiz == 0 || vert == 0) return 0;

	while(selectedX < 0) selectedX += (horiz > 0) ? horiz : 1;
	while(selectedY < 0) selectedY += vert;
	selectedX %= horiz;
	selectedY %= vert;

	width = horiz*(iconRes+2*iconSpacing);
	height = vert*(iconRes+2*iconSpacing);

	HDC hDC = GetDC(NULL);
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hMemBmp = CreateCompatibleBitmap(hDC, width, height);
	HGDIOBJ hOrgBMP = SelectObject(hMemDC, hMemBmp);

	// draw icons
	int count = 0;
	applicationList.clear();
	for(int p = 0; p < paths.size(); ++p) {
		const char *path_ = paths[p].c_str();
		std::vector<std::string> appDirectory;
		if(Filesystem::isDirectory(path_) && std::string(path_).find(".lnk") == std::string::npos && std::string(path_).find(".exe") == std::string::npos) {
			appDirectory = Filesystem::listDirectory(path_);
		} else {
			appDirectory.push_back(path_);
			std::cerr << path_ << std::endl;
		}

		for(int i = 0; i < appDirectory.size(); ++i) {
			if(appDirectory[i].find(".lnk") == std::string::npos && appDirectory[i].find(".exe") == std::string::npos) continue;
			const int x = (count/vert);
			const int y = (vert-(count%vert)-1);
			const int yApp = (count%vert);

			const std::string s(Filesystem::getFullPath(std::string(path_), appDirectory[i].c_str()));
			const char *path = s.c_str();

			applicationList[std::pair<int,int>(x,yApp)] = s;

			// std::cerr << "***** ICON FOR: " << path << std::endl;
			std::wstring wpath(s.begin(), s.end());
			std::cerr << wpath.c_str() << std::endl;
			SHFILEINFOW sfi = {0};
			HRESULT hr = SHGetFileInfo(wpath.data(),
				-1,//attr,
				&sfi,
				sizeof(SHFILEINFO), 
				SHGFI_ICON | /*SHGFI_SMALLICON | SHGFI_TYPENAME | */SHGFI_DISPLAYNAME);//SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_TYPENAME);

			if(!SUCCEEDED(hr))
			{
				continue;
			}
			// sfi.szDisplayName for name to display as text
			drawIconToDC(sfi.hIcon, hMemDC, x*(iconRes+2*iconSpacing)+0.5*iconSpacing, y*(iconRes+2*iconSpacing)+0.5*iconSpacing);

			++count;
		}
	}
	// draw frame
	static HPEN hPen = CreatePen(PS_SOLID,5,RGB(255,0,0));
	::SelectObject(hMemDC, ::GetStockObject(NULL_BRUSH)); // transparent brush
	SelectObject(hMemDC,  hPen);
	Rectangle(hMemDC, selectedX*(iconRes+2*iconSpacing), (vert-selectedY-1)*(iconRes+2*iconSpacing), selectedX*(iconRes+2*iconSpacing)+iconRes+2*iconSpacing, (vert-selectedY-1)*(iconRes+2*iconSpacing)+iconRes+2*iconSpacing);


	BITMAPINFOHEADER   bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);    
	bi.biWidth = width;
	bi.biHeight = height;
	bi.biPlanes = 1;    
	bi.biBitCount = 24;    
	bi.biCompression = BI_RGB;    
	bi.biSizeImage = 0;  
	bi.biXPelsPerMeter = 0;    
	bi.biYPelsPerMeter = 0;    
	bi.biClrUsed = 0;    
	bi.biClrImportant = 0;

	if(lpbitmap != 0) {
		delete [] lpbitmap;
		lpbitmap = 0;
	}
	if(lpbitmap == 0) {
		DWORD dwBmpSize = ((width * bi.biBitCount + 23) / 24) * 3 *height;
		lpbitmap = new char[dwBmpSize];
	}


	// Gets the "bits" from the bitmap and copies them into a buffer 
	// which is pointed to by lpbitmap.
	int result = GetDIBits(hMemDC, hMemBmp, 0,
		height,
		lpbitmap,
		(BITMAPINFO *)&bi, DIB_RGB_COLORS);

	if(appTexture == 0) {
		glGenTextures(1, &appTexture);
	}
	if(appTexture) {
		glBindTexture(GL_TEXTURE_2D, appTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, lpbitmap);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	}


	DeleteObject(hMemBmp);
	SelectObject(hMemDC, hOrgBMP);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hDC);

	return appTexture;
}
#endif

void Ibex::ApplicationLauncher::update() {
	if(appSelectionTexture == 0) {
		appSelectionTexture = loadTexture("/resources/app-launcher-selection-frame.png");
	}
	if(appTexture == 0 || newX != selectedX || newY != selectedY) {
		if(appTexture) {
			glDeleteTextures(1, &appTexture);
			appTexture = 0;
		}
		std::vector<std::string> directories;
#ifdef __APPLE__
		//directories.push_back("/Applications");
		directories = getApplicationDirectoryFromPreferences();
		appTexture = createApplicationListImage(directories, ww, hh, newX, newY, applicationList);
#endif
#ifdef WIN32
		wchar_t directoriesString[20000];
		GetPrivateProfileString(L"appLauncher", L"directories",  TEXT(""), directoriesString, 20000, L".\\ibex.ini");
		std::wstring wString(directoriesString);
		std::string directoryPlainString(wString.begin(), wString.end());
		std::istringstream iss(directoryPlainString);
		std::string string;
		while(std::getline( iss,string, ';'))
        {
			directories.push_back(string);
        }
		//directories.push_back("%AppData%\\Microsoft\\Windows\\Start Menu\\Programs");
		//directories.push_back("%ProgramData%\\Microsoft\\Windows\\Start Menu\\Programs");
		createApplicationLauncherBitmap(directories, ww, hh, newX, newY, applicationList);
#endif
		selectedX = newX;
		selectedY = newY;
	}
}

void Ibex::ApplicationLauncher::render(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP)
{
#if !defined(__APPLE__) && !defined(WIN32)
	showApplicationLauncher = false;
	return;
#endif

	static GLuint vaoIbexDisplayFlat = 0;
	static const GLfloat IbexDisplayFlatScale = 10;

	static GLint IbexDisplayFlatUniformLocations[7] = { 0, 0, 0, 0, 0, 0, 0};
	static GLint IbexDisplayFlatAttribLocations[3] = { 0, 0, 0 };

	static GLfloat IbexDisplayFlatVertices[] = {
		-1.0,  -1, 0.0, 0, 0, -1, 0, 0,
		1.0, -1.0, 0.0, 0, 0, -1, 1, 0,
		1.0, 1.0, 0.0, 0, 0, -1, 1, 1,
		-1.0, 1.0, 0.0, 0, 0, -1, 0, 1,
	};
	static GLuint vboIbexDisplayFlatVertices = 0;

	static GLushort IbexDisplayFlatIndices[] = {
		0, 1, 2,
		0, 2, 3
	};
	static GLuint vboIbexDisplayFlatIndices = 0;

	if(first) {
		first = false;

		for(int i = 0; i < sizeof(IbexDisplayFlatVertices)/sizeof(GLfloat); ++i) {
			if(i%8 < 3)
				IbexDisplayFlatVertices[i] *= IbexDisplayFlatScale;
			if(ww != 0 && ww > hh) {
				if(i%8 == 1)
					IbexDisplayFlatVertices[i] *= (ww != 0) ? float(hh)/float(ww) : 1.0;
			} else {
				if(i%8 == 0)
					IbexDisplayFlatVertices[i] *= (hh != 0) ? float(ww)/float(hh) : 1.0;
			}
		}

		if(standardShaderProgram.shader.program == 0) standardShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/emissive.v.glsl", "/resources/shaders/emissive.f.glsl");
		glUseProgram(standardShaderProgram.shader.program);


		IbexDisplayFlatUniformLocations[0] = glGetUniformLocation(standardShaderProgram.shader.program, "MVP");
		IbexDisplayFlatUniformLocations[1] = glGetUniformLocation(standardShaderProgram.shader.program, "V");
		IbexDisplayFlatUniformLocations[2] = glGetUniformLocation(standardShaderProgram.shader.program, "M");
		IbexDisplayFlatUniformLocations[3] = glGetUniformLocation(standardShaderProgram.shader.program, "textureIn");
		IbexDisplayFlatUniformLocations[4] = glGetUniformLocation(standardShaderProgram.shader.program, "MV");
		IbexDisplayFlatUniformLocations[5] = glGetUniformLocation(standardShaderProgram.shader.program, "inFade");
		IbexDisplayFlatUniformLocations[6] = glGetUniformLocation(standardShaderProgram.shader.program, "offset");

		IbexDisplayFlatAttribLocations[0] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexPosition_modelspace");
		IbexDisplayFlatAttribLocations[1] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexNormal_modelspace");
		IbexDisplayFlatAttribLocations[2] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexUV");

		glUseProgram(0);

		std::cerr << "setup_buffers" << std::endl;
		checkForErrors();
		if(vaoIbexDisplayFlat == 0) glGenVertexArrays(1,&vaoIbexDisplayFlat);

		checkForErrors();
		std::cerr << "gen vaoIbexDisplayFlat done" << std::endl;

		glBindVertexArray(vaoIbexDisplayFlat);
		if(vboIbexDisplayFlatVertices == 0) glGenBuffers(1, &vboIbexDisplayFlatVertices);
		glBindBuffer(GL_ARRAY_BUFFER, vboIbexDisplayFlatVertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(IbexDisplayFlatVertices), IbexDisplayFlatVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[0]);
		glVertexAttribPointer(IbexDisplayFlatAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, 0);
		glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[2]);
		glVertexAttribPointer(IbexDisplayFlatAttribLocations[2], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 6));


		if(vboIbexDisplayFlatIndices == 0) glGenBuffers(1, &vboIbexDisplayFlatIndices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIbexDisplayFlatIndices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IbexDisplayFlatIndices), IbexDisplayFlatIndices, GL_STATIC_DRAW);
	}

	if(shadowPass) {
		glUseProgram(shadowProgram.shader.program);
		glUniformMatrix4fv(ShadowUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
	} else {
		glUseProgram(standardShaderProgram.shader.program);
		glUniformMatrix4fv(IbexDisplayFlatUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(IbexDisplayFlatUniformLocations[1], 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(IbexDisplayFlatUniformLocations[2], 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(IbexDisplayFlatUniformLocations[4], 1, GL_FALSE, &(V*M)[0][0]);

		if(IbexDisplayFlatUniformLocations[5] >= 0) glUniform1f(IbexDisplayFlatUniformLocations[5], 1.0);
		if(IbexDisplayFlatUniformLocations[6] >= 0) {
			glUniform2f(IbexDisplayFlatUniformLocations[6], 0,0);
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, appTexture);
		glUniform1i(IbexDisplayFlatUniformLocations[3], 0);
	}

	glBindVertexArray(vaoIbexDisplayFlat);
	glDrawElements(GL_TRIANGLES, sizeof(IbexDisplayFlatIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
}

#ifdef __APPLE__
int Ibex::ApplicationLauncher::processKey(unsigned short keyCode, int down) {
	int processed = 0;

	switch(keyCode) {
	case kVK_UpArrow:
	case kVK_ANSI_W:
		if(down) {
			newY = selectedY-1;
		}
		processed = 1;
		break;
	case kVK_DownArrow:
	case kVK_ANSI_S:
		if(down) {
			newY = selectedY+1;
		}
		processed = 1;
		break;
	case kVK_LeftArrow:
	case kVK_ANSI_A:
		if(down) {
			newX = selectedX-1;
		}
		processed = 1;
		break;
	case kVK_RightArrow:
	case kVK_ANSI_D:
		if(down) {
			newX = selectedX+1;
		}
		processed = 1;
		break;
	case kVK_Return:
		if(down) {
			std::string path = applicationList[std::pair<int,int>(selectedX,selectedY)];
			if(path != "") {
				launchApplication(path.c_str());
				showApplicationLauncher = false;
			}
		}
		processed = 1;
		break;
	case kVK_Escape:
		showApplicationLauncher = false;
		processed = 1;
		break;
	}
	return processed;
}
#else
#ifdef WIN32
int Ibex::ApplicationLauncher::processKey(int key, int down) {
	int processed = 0;
	switch(key) {
	case 'W':
	case 'w':
	case GLFW_KEY_UP:
		if(down) {
			newY = selectedY-1;
		}
		processed = 1;
		break;
	case GLFW_KEY_DOWN:
	case 'S':
	case 's':
		if(down) {
			newY = selectedY+1;
		}

		processed = 1;
		break;
	case 'D':
	case 'd':
	case GLFW_KEY_RIGHT:
		if(down) {
			newX = selectedX+1;
		}
		processed = 1;
		break;
	case GLFW_KEY_LEFT:
	case 'A':
	case 'a':
		if(down) {
			newX = selectedX-1;
		}

		processed = 1;
		break;
	case GLFW_KEY_ENTER:
		if(!down) {
			std::string path = applicationList[std::pair<int,int>(selectedX,selectedY)];
			if(path != "") {
				launchApplication(path.c_str());
				showApplicationLauncher = false;
			}
		}
		processed = 1;
		break;
		// case 27: // ESCAPE
	case GLFW_KEY_ESCAPE:
		if(!down) {
			showApplicationLauncher = false;
		}

		processed = 1;
	}
	return processed;
}
#else
int Ibex::ApplicationLauncher::processKey(XIDeviceEvent *event, bool down) {
	int processed = 0;
	return processed;
}
#endif
#endif
