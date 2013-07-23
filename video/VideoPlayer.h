//
//  VideoPlayer.h
//  IbexMac
//
//  Created by Hesham Wahba on 4/30/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__VideoPlayer__
#define __IbexMac__VideoPlayer__

extern "C" {
#ifndef _WIN32

#ifdef __APPLE__

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#else

#include <AL/al.h>
#include <AL/alc.h>

#endif

#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif

#include <stdint.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/avfiltergraph.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
  //#include <libavutil/time.h>
}

#include <list>
#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

struct CvCapture;

namespace Ibex {
    
#ifdef _WIN32
class WindowsAudioSource;
#endif

struct AudioPacket {
    AVFrame *avAudioFrame;
    uint8_t *audioBuffer;
    int size;
    
    //int64_t pts;
    int64_t dts;
    double pts;
};

enum VideoSyncMode {
    SyncVideo,SyncAudio,SyncExternal
};

class VideoPlayer {
#ifdef _WIN32
	friend class WindowsAudioSource;
#endif

public:
    VideoPlayer();
	~VideoPlayer();
    
    void savePPMFrame(const AVFrame *pFrame, int width, int height, int iFrame) const;
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

    void addAudioFrame(AudioPacket avAudioFrame);
    void addVideoFrame(AudioPacket avAudioFrame);
    int loadSyncAudioVideo(const char *fileName_, bool isStereo);
    int playAudio(AVCodecContext *avAudioCodecCtx);
    int initVideo(const char *fileName_, bool isStereo);
    
    double getGlobalVideoPTS(AVFrame *src_frame, double pts);
    
    VideoSyncMode videoSyncMode;
    
    bool done;
    bool videoDone;
    bool audioDone;
    
    std::queue<AudioPacket> videoQueue;
    std::queue<AVFrame*> videoFrameQueue;

    std::mutex aMutex1;
    std::mutex aMutex2;
    std::queue<AudioPacket> audioQueue;
    std::queue<AudioPacket> audioBufferQueue;
    
    AVStream *avVideoStream;
    AVStream *avAudioStream;
    
private:
    double videoClock;
    double audioClock;
    double synchronize_video(AVFrame *src_frame, double pts);
    double getSyncClock();
    
private:
    AVFormatContext *avFormatCtx;
    int             videoStream, audioStream;
    AVCodecContext  *avCodecCtx;
    AVCodecContext  *avAudioCodecCtx;
    AVCodec         *avCodec;
    AVCodec         *avAudioCodec;
    AVFrame         *avFrame;
    AVFrame         *avAudioFrame;
    AVFrame         *avFrameRGB;
    AVPacket        avPacket;
    int             gotCompletePictureFrame,gotCompleteAudioFrame;
    int             numBytes;
    uint8_t         *buffer;

    bool openCVInited;
    bool captureVideo;
    CvCapture *cvCapture;
    
private:
    std::thread syncThread;
    std::thread audioThread;
};
    
}

#endif /* defined(__IbexMac__VideoPlayer__) */
