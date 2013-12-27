//
//  Window.h
//  IbexMac
//
//  Created by Hesham Wahba on 5/7/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__Window__
#define __IbexMac__Window__

#if !defined(__APPLE__) && !defined(WIN32)
#include "../x11/x11.h"
#endif

#ifdef WIN32
typedef unsigned int uint;
#endif

#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <set>

#include "TextRenderer.h"

namespace Ibex {
    
enum VisibleWindow {
    NoWindow,InfoWindow,FileChooser,CameraChooser
};

class Window {
public:
    Window();
    
	void reset();
    void render(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);
    void update(double timeDelta);
    bool getIsStereoVideo() { return isStereoVideo; }
    bool getSelectedVideo() { return selectedVideo; }
    bool getSelectedCamera() { return selectedCamera; }
    bool setSelectedVideo(bool selectedVideo_) { selectedVideo = selectedVideo_; return selectedVideo; }
    bool setSelectedCamera(bool selectedCamera_) { selectedCamera = selectedCamera_; return selectedCamera; }
    std::string getSelectedVideoPath() { return videoPath; }
    uint getSelectedCameraID() { return selectedCameraID; }
    void showDialog(bool showDialog_) { visibleWindow = (showDialog_) ? InfoWindow : NoWindow; };

#ifdef __APPLE__
    int processKey(unsigned short keyCode, int down);
#else
#ifdef WIN32
    int processKey(int key, int down);
    int processSpecialKey(int key, int down);
#else
    int processKey(XIDeviceEvent *event, bool pressed);
#endif
#endif
    
private:
    void renderFileChooser();
    void renderCameraChooser();
    void renderInfoWindow();
    
private:
    bool selectedVideo;
    bool isStereoVideo;
public:
    std::string videoPath;
    
private:
    VisibleWindow visibleWindow;
    VisibleWindow previousVisibleWindow;
    std::string currentPath;
    bool directoryChanged;
    uint selectedFile;
    uint selectedCamera;
    uint selectedCameraID;
    std::vector<std::string> directoryList;
    std::vector<int> cameras;
    std::unordered_set<std::string> fileTypes;
    
    bool updateRender;
    ::Ibex::TextRenderer *textRenderer;
};
    
}

#endif /* defined(__IbexMac__Window__) */
