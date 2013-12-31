//
//  VideoPlayer.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 4/30/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "VLCVideoPlayer.h"

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

#if !defined(__APPLE__) && !defined(WIN32)
#include "../opengl_setup_x11.h"
#endif

#ifdef WIN32
void(*makeCurrentGL)();
#endif

static unsigned int VIDEOWIDTH=1280;
static unsigned int VIDEOHEIGHT=720;

void setupVideoGLContext(void *data);

// based off of http://wiki.videolan.org/index.php?title=LibVLC_SampleCode_SDL&redirect=no which was licensed under the WTFPL license http://en.wikipedia.org/wiki/WTFPL 

struct context {
  // textures or whatever can go here
  Ibex::VLCVideoPlayer *player;
  unsigned int *videoTexture;
  bool isStereo;
  Display *dpy;
  GLXDrawable root;
    void *data;
  int n;
    libvlc_media_player_t *mp;
	bool init;
	context() : init(false) {
	}
};

static uint32_t *pixels;
// VLC prepares to render a video frame.
static void *vlclock(void *data, void **p_pixels) {
  // lock mutex for texture update
  *p_pixels = pixels;

  return NULL; // Picture identifier, not needed here.
}

// VLC just rendered a video frame.
static void vlcunlock(void *data, void *id, void *const *p_pixels) {
    // unlock texture update mutex
    assert(id == NULL); // picture identifier, not needed here
}

// VLC wants to display a video frame.
static void vlcdisplay(void *data, void *id) {
	try {
  struct context *c = (struct context*)data;
  
  unsigned int *videoTexture = c->videoTexture;
  bool isStereo = c->isStereo;
  //std::cerr << "**** display: " << videoTexture[0] << std::endl;

//setup GL context by OS
#if __APPLE__
    static bool init2 = false;
    if(!init2) {
        init2 = true;
        setupVideoGLContext(c->data);
    }
#else
#ifdef WIN32
  makeCurrentGL = (void(*)())c->data;
  makeCurrentGL();
#else
 // Create the pixmap, where one designs the scene
  static Pixmap pix = XCreatePixmap(c->dpy, c->root, VIDEOWIDTH, VIDEOHEIGHT, vi->depth);
  static GLXPixmap px = glXCreateGLXPixmap(c->dpy, vi, pix);

  static GLXContext ctx = glXCreateContext(c->dpy, vi, context, GL_TRUE);
  static bool r = glXMakeCurrent(c->dpy, px, ctx);
  if(!r) {
    std::cerr << "* failed to create GL for video" << std::endl;
  }
#endif
#endif
    
  if(!c->init) {
        c->init = true;
      
        c->player->createVideoTextures(c->isStereo, VIDEOWIDTH, VIDEOHEIGHT);
        c->player->width = VIDEOWIDTH;
        c->player->height = VIDEOHEIGHT;
          
        unsigned int width,height;
        if(libvlc_video_get_size(c->mp, 0, &width, & height) == 0) {
          c->player->width = width;
          c->player->height = height;
            std::cerr << "***************** " << VIDEOWIDTH << "x" << VIDEOHEIGHT << " ==> " << width << "x" << height << std::endl;
        }
    
        isStereo = false;
  }

  const GLuint width = VIDEOWIDTH;
  const GLuint height = VIDEOHEIGHT;
  static bool first = true;
  if(isStereo) {
#if __APPLE__
#define IBEX_VIDEO_GL_PIX_FORMAT GL_UNSIGNED_INT_8_8_8_8_REV
#else
#define IBEX_VIDEO_GL_PIX_FORMAT GL_UNSIGNED_BYTE
#endif
    glBindTexture(GL_TEXTURE_2D, videoTexture[1]);
    int stride = width*2;
    glPixelStorei(GL_UNPACK_ROW_LENGTH,stride);
    if(first) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height/2, 0,
		   GL_BGRA, IBEX_VIDEO_GL_PIX_FORMAT, pixels);
      glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height/2, 0,
		   GL_BGRA, IBEX_VIDEO_GL_PIX_FORMAT, pixels+(width*3));
    } else {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height/2,
		      GL_BGRA, IBEX_VIDEO_GL_PIX_FORMAT, pixels);
      glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height/2,
		      GL_BGRA, IBEX_VIDEO_GL_PIX_FORMAT , pixels+(width*3));
    }
  } else {
    glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
    if(first) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
      	   GL_BGRA, IBEX_VIDEO_GL_PIX_FORMAT, pixels);
    } else {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
		      GL_BGRA, IBEX_VIDEO_GL_PIX_FORMAT, pixels);
    }
  }
  first = false;
  glFlush();

  //struct context *c = (context *)data;
    // WIDTH/HEIGHT VIDEOWIDTH/VIDEOHEIGHT
	}catch(...) {
	}
}

static void quit(int c) {
    exit(c);
}

int Ibex::VLCVideoPlayer::playVLCVideo(const char *fileName, Display *dpy, GLXDrawable root) {
	if(mp != 0) {
		try {
			pixels = 0;
			done = true;

			// Stop stream and clean up libVLC.
			libvlc_media_player_stop(mp);
			libvlc_media_player_release(mp);
			mp = 0;
		} catch(...) {
			mp = 0;
		}
	}

    char const *vlc_argv[] = {
        //"-H"
        "--text-renderer","none"
        //"--reset-plugins-cache",
        //"--reset-config"
      //"--no-audio", // Don't play audio.
       //"--no-xlib" // Don't use Xlib.

        // Apply a video filter.
        //"--video-filter", "sepia",
        //"--sepia-intensity=200"
    };
    int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
	
    int action = 0, pause = 0;
    context = new struct context();
    context->isStereo = 0;
    context->videoTexture = videoTexture;
    context->player = this;
    context->dpy = dpy;
    context->root = root;
    context->data = data;

    // create texture/mutex
    //setenv("VLC_PLUGIN_PATH", "/Applications/VLC.app/Contents/MacOS/plugins", 1);
    // If you don't have this variable set you must have plugins directory
    // with the executable or libvlc_new() will not work!
    printf("VLC_PLUGIN_PATH=%s\n", getenv("VLC_PLUGIN_PATH"));

    // Initialise libVLC.
	if(libvlc == 0) {
		libvlc = libvlc_new(vlc_argc, vlc_argv);
		if(NULL == libvlc) {
			printf("LibVLC initialization failure.\n");
			return EXIT_FAILURE;
		}
	}

    libvlc_media_t *m = libvlc_media_new_path(libvlc, fileName);
    mp = libvlc_media_player_new_from_media(m);
    libvlc_media_release(m);
	m = 0;

    pixels = new uint32_t[VIDEOWIDTH*VIDEOHEIGHT*4];
    memset(pixels, 0, sizeof(uint32_t)*VIDEOWIDTH*VIDEOHEIGHT*4);
    libvlc_video_set_callbacks(mp, vlclock, vlcunlock, vlcdisplay, context);
    libvlc_video_set_format(mp, "RV32", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH*sizeof(uint32_t));
    //libvlc_video_set_format(mp, "YUYV", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH*2);
    
    if(libvlc_video_get_size(mp, 0, &width, & height) == 0) {
        VIDEOWIDTH = width;
        VIDEOHEIGHT = height;
    }
    context->mp = mp;
    
	done = false;
    libvlc_media_player_play(mp);

    // Main loop.
    while(!done) {
        action = 0;
        switch(action) {
			case 's':
				libvlc_media_player_stop(mp);
				break;
            case 'q':
                done = 1;
                break;
            case ' ':
                printf("Pause toggle.\n");
                pause = !pause;
                break;
        }

        if(!pause) { context->n++; }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

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
									cvCapture(0),
									libvlc(0),
									mp(0),
									context(0) {
  videoTexture[0] = videoTexture[1] = 0;
}
Ibex::VLCVideoPlayer::~VLCVideoPlayer() {
  stopCapturing();
  stopPlaying();

  if(mp) {
    // Stop stream and clean up libVLC.
	while(libvlc_media_player_get_state(mp) != libvlc_Stopped) {
		libvlc_media_player_stop(mp);
	}
    libvlc_media_player_release(mp);
	mp = 0;
  }
  if(libvlc) {
    libvlc_release(libvlc);
	libvlc = 0;
  }
  if(context) {
	  delete context;
	  context = 0;
  }
}

int Ibex::VLCVideoPlayer::playVideo(const char *fileName, bool isStereo, Display *dpy, GLXDrawable root, const void *data)
{
    this->data = (void*)data;
	//createVideoTextures(isStereo, VIDEOWIDTH, VIDEOHEIGHT);
	this->isStereo = isStereo;
	playVLCVideo(fileName, dpy, root);
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
  done = false;
  if(!openCVInited) {
    initOpenCV(isStereo, cameraId);
  }
  
  bool first = true;
  while(captureVideo && cvCapture && !done) {
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
    if (!checkForErrors()) {
        fprintf(stderr,"Stage 00 - Problem generating videoTexture FBO");
        exit(EXIT_FAILURE);
    }
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
