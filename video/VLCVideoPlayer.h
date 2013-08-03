//
//  VLCVideoPlayer.h
//  IbexMac
//
//  Created by Hesham Wahba on 4/30/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef __VLCVideoPlayer__
#define __VLCVideoPlayer__

#include <list>
#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

// --- OpenGL ----------------------------------------------------------------
#define GLX_GLXEXT_PROTOTYPES
#include <X11/Xlib.h>
#include <X11/Xregion.h>
#include <X11/Xresource.h>
#include <X11/X.h>

#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>
#include <GL/glu.h>

struct CvCapture;

namespace Ibex {
    

class VLCVideoPlayer {
public:
    VLCVideoPlayer();
    ~VLCVideoPlayer();
    
    int playVideo(const char *fileName, bool isStereo, Display *dpy, GLXDrawable root);

    int openCamera(bool isStereo, int cameraId);
    static std::vector<int> listCameras();
    void stopCapturing();
    void stopPlaying();
    
public:
    unsigned int *videoTexture;
    unsigned int width,height;
    
    void createVideoTextures(bool isStereo, int width, int height);
private:
    void initOpenCV(bool isStereo, int cameraId);
    int initVideo(const char *fileName_, bool isStereo);
    
    bool done;
    bool isStereo;
    bool videoDone;
    bool audioDone;

    
private:
    bool openCVInited;
    bool captureVideo;
    CvCapture *cvCapture;
    
private:
    std::thread syncThread;
    std::thread audioThread;

 private:
    int playVLCVideo(const char *fileName, Display *dpy, GLXDrawable root);
};
    
}

#endif /* defined(__VLCVideoPlayer__) */
