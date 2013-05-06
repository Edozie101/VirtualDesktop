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
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
    
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
    
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/avfiltergraph.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#include <queue>

namespace Ibex {
    
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
public:
    VideoPlayer();
    
    void savePPMFrame(const AVFrame *pFrame, int width, int height, int iFrame) const;
    int playVideo(const char *fileName, bool isStereo);
    
public:
    unsigned int *videoTexture;
    unsigned int width,height;
    
private:
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
    
    std::queue<AudioPacket> audioQueue;
    std::queue<AudioPacket> videoQueue;
    std::queue<AVFrame*> videoFrameQueue;
    std::queue<AudioPacket> audioBufferQueue;
    
    AVStream *avVideoStream;
    AVStream *avAudioStream;
    
private:
    double videoClock;
    double audioClock;
    double synchronize_video(AVFrame *src_frame, double pts);
    double getSyncClock();
    
private:
    AVFormatContext *avFormatCtx = NULL;
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
};
    
}

#endif /* defined(__IbexMac__VideoPlayer__) */
