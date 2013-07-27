//
//  VideoPlayer.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 4/30/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//
#include "VLCVideoPlayer.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "../opengl_helpers.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <stdint.h>


#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "vlc/vlc.h"

#define WIDTH 640
#define HEIGHT 480

#define VIDEOWIDTH 320
#define VIDEOHEIGHT 240

// based off of http://wiki.videolan.org/index.php?title=LibVLC_SampleCode_SDL&redirect=no which was licensed under the WTFPL license http://en.wikipedia.org/wiki/WTFPL 

struct context {
  // textures or whatever can go here
  int n;
};

// VLC prepares to render a video frame.
static void *lock(void *data, void **p_pixels) {
  //struct context *c = (context *)data;

  //int pitch;
    // lock mutex for texture update

    return NULL; // Picture identifier, not needed here.
}

// VLC just rendered a video frame.
static void unlock(void *data, void *id, void *const *p_pixels) {

  //struct context *c = (context *)data;

    uint16_t *pixels = (uint16_t *)*p_pixels;

    // We can also render stuff.
    int x, y;
    for(y = 10; y < 40; y++) {
        for(x = 10; x < 40; x++) {
            if(x < 13 || y < 13 || x > 36 || y > 36) {
                pixels[y * VIDEOWIDTH + x] = 0xffff;
            } else {
                // RV16 = 5+6+5 pixels per color, BGR.
                pixels[y * VIDEOWIDTH + x] = 0x02ff;
            }
        }
    }

    // unlock texture update mutex
    assert(id == NULL); // picture identifier, not needed here
}

// VLC wants to display a video frame.
static void display(void *data, void *id) {
  //struct context *c = (context *)data;
    // WIDTH/HEIGHT VIDEOWIDTH/VIDEOHEIGHT
}

static void quit(int c) {
    exit(c);
}

static int main_test(int argc, char *argv[]) {

    libvlc_instance_t *libvlc;
    libvlc_media_t *m;
    libvlc_media_player_t *mp;
    char const *vlc_argv[] = {

        "--no-audio", // Don't play audio.
        "--no-xlib", // Don't use Xlib.

        // Apply a video filter.
        //"--video-filter", "sepia",
        //"--sepia-intensity=200"
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);

    int done = 0, action = 0, pause = 0;
    struct context context;

    if(argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
   
    // create texture/mutex

    // If you don't have this variable set you must have plugins directory
    // with the executable or libvlc_new() will not work!
    printf("VLC_PLUGIN_PATH=%s\n", getenv("VLC_PLUGIN_PATH"));

    // Initialise libVLC.
    libvlc = libvlc_new(vlc_argc, vlc_argv);
    if(NULL == libvlc) {
        printf("LibVLC initialization failure.\n");
        return EXIT_FAILURE;
    }

    m = libvlc_media_new_path(libvlc, argv[1]);
    mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release(m);

    libvlc_video_set_callbacks(mp, lock, unlock, display, &context);
    libvlc_video_set_format(mp, "RV16", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH*2);
    libvlc_media_player_play(mp);
    
    // Main loop.
    while(!done) {
        action = 0;
        switch(action) {
            case 'q':
                done = 1;
                break;
            case ' ':
                printf("Pause toggle.\n");
                pause = !pause;
                break;
        }

        if(!pause) { context.n++; }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Stop stream and clean up libVLC.
    libvlc_media_player_stop(mp);
    libvlc_media_player_release(mp);
    libvlc_release(libvlc);

    // delete mutex/texture info

    quit(0);

    return 0;
}


Ibex::VLCVideoPlayer::VLCVideoPlayer() :  videoTexture(new unsigned int[2]),
                                    width(0),
                                    height(0),
                                    done(true),
                                    videoDone(true),
                                    audioDone(true),
				    openCVInited(false),
				    captureVideo(false),
				    cvCapture(0) {
  videoTexture[0] = videoTexture[1] = 0;
}
Ibex::VLCVideoPlayer::~VLCVideoPlayer() {
  stopCapturing();
  stopPlaying();
}

int Ibex::VLCVideoPlayer::playVideo(const char *fileName, bool isStereo)
{
  return 0;
}

std::vector<int> Ibex::VLCVideoPlayer::listCameras() {
  std::vector<int> cameras;
    
  CvCapture *cam;
  int i = 0;
  do
    {
      cam = cvCreateCameraCapture(i++);
      cameras.push_back(i-1);
      if(cam == NULL) break;
      cvReleaseCapture(&cam);
    } while(true && i < 5);
  if(cam) cvReleaseCapture(&cam);
    
  return cameras;
}

void Ibex::VLCVideoPlayer::stopCapturing() {
  done = true;
  captureVideo = false;
  if(cvCapture) {
    cvReleaseCapture(&cvCapture);
    cvCapture = NULL;
    openCVInited = false;
  }
}
void Ibex::VLCVideoPlayer::stopPlaying() {
  done = true;
}

void Ibex::VLCVideoPlayer::initOpenCV(bool isStereo, int cameraId) {
  if(!openCVInited) {
    captureVideo = true;
    cvCapture = cvCaptureFromCAM(cameraId);//cvCreateCameraCapture(cameraId);
    width = cvGetCaptureProperty(cvCapture, CV_CAP_PROP_FRAME_WIDTH);
    height = cvGetCaptureProperty(cvCapture, CV_CAP_PROP_FRAME_HEIGHT);
    createVideoTextures(isStereo, width, height);
    openCVInited = true;
  }
}

int Ibex::VLCVideoPlayer::openCamera(bool isStereo, int cameraId) {
  if(!openCVInited) {
    initOpenCV(isStereo, cameraId);
  }
  
  bool first = true;
  while(captureVideo && cvCapture) {
    IplImage* cameraCapture = cvQueryFrame(cvCapture);
        
    if( cameraCapture && (cameraCapture->width > 0) && (cameraCapture->height > 0)) {
      //width = cameraCapture->width;
      //height = cameraCapture->height;
      const int numBytes = (cameraCapture->widthStep/cameraCapture->width == 4 || cameraCapture->nChannels == 4) ? 4 : 3;
      const GLenum formatIn = (numBytes == 4) ? GL_BGRA : GL_BGR;
      if(isStereo) {
	glBindTexture(GL_TEXTURE_2D, videoTexture[1]);
	const int stride = width*2;
	glPixelStorei(GL_UNPACK_ROW_LENGTH,stride);
	if(first) {
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height/2, 0,
		       formatIn, GL_UNSIGNED_BYTE, cameraCapture->imageData);
	  glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height/2, 0,
		       formatIn, GL_UNSIGNED_BYTE, cameraCapture->imageData+(width*numBytes));
	} else {
	  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height/2,
			  formatIn, GL_UNSIGNED_BYTE, cameraCapture->imageData);
	  glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
	  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height/2,
			  formatIn, GL_UNSIGNED_BYTE, cameraCapture->imageData+(width*numBytes));
	}
      } else {
	glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
	const int stride = width;
	glPixelStorei(GL_UNPACK_ROW_LENGTH,stride);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	if(first) {
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
		       formatIn, GL_UNSIGNED_BYTE, cameraCapture->imageData);
	} else {
	  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
			  formatIn, GL_UNSIGNED_BYTE, cameraCapture->imageData);
	}
      }
      glFlush();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }
  stopCapturing();
    
  return 0;
}

void Ibex::VLCVideoPlayer::createVideoTextures(bool isStereo, int width, int height) {
  // load OpenGL textures for video/stereo-video if necessary
  if(videoTexture[0]) {
    if(videoTexture[1] == videoTexture[0]) {
      glDeleteTextures(1, videoTexture);
    } else {
      glDeleteTextures(2, videoTexture);
    }
    videoTexture[0] = videoTexture[1] = 0;
  }
  glGenTextures((isStereo) ? 2 : 1, videoTexture);
  for(int i = 0; i < 2; ++i) {
    if(i == 1 && !isStereo) {
      videoTexture[i] = videoTexture[0];
      break;
    }
        
    glBindTexture(GL_TEXTURE_2D, videoTexture[i]);
    if (!checkForErrors()) {
      fprintf(stderr,"Stage 0a - Problem generating videoTexture FBO");
      exit(EXIT_FAILURE);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (!checkForErrors()) {
      fprintf(stderr,"Stage 0b - Problem generating videoTexture FBO");
      exit(EXIT_FAILURE);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    if (!checkForErrors()) {
      fprintf(stderr,"Stage 0c - Problem generating videoTexture FBO");
      exit(EXIT_FAILURE);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
		 GL_RGB, GL_UNSIGNED_BYTE, 0);
    if (!checkForErrors()) {
      fprintf(stderr,"Stage 0d - Problem generating videoTexture FBO");
      exit(EXIT_FAILURE);
    }
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glFlush();
}
