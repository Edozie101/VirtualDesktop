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

struct CvCapture;

namespace Ibex {
    

class VLCVideoPlayer {
public:
    VLCVideoPlayer();
    ~VLCVideoPlayer();
    
    int playVideo(const char *fileName, bool isStereo);

    int openCamera(bool isStereo, int cameraId);
    static std::vector<int> listCameras();
    void stopCapturing();
    void stopPlaying();
    
public:
    unsigned int *videoTexture;
    unsigned int width,height;
    
private:
    void createVideoTextures(bool isStereo, int width, int height);
    void initOpenCV(bool isStereo, int cameraId);
    int initVideo(const char *fileName_, bool isStereo);
    
    bool done;
    bool videoDone;
    bool audioDone;

    
private:
    bool openCVInited;
    bool captureVideo;
    CvCapture *cvCapture;
    
private:
    std::thread syncThread;
    std::thread audioThread;
};
    
}

#endif /* defined(__VLCVideoPlayer__) */
