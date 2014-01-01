//
//  Window.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 5/7/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "Window.h"
#include "../opengl_helpers.h"

#include "../ibex.h"
#include "../filesystem/Filesystem.h"
#include <algorithm>
#include <sstream>
#include <string>
#include "../video/VLCVideoPlayer.h"

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#ifdef WIN32
#define fmin min
#endif

#define NUM_LINES 12

bool endsWith(std::string const &inputString, std::string const &ending)
{
    if (inputString.length() >= ending.length())
        return (inputString.compare(inputString.length() - ending.length(), ending.length(), ending) == 0);
    else
        return false;
}

Ibex::Window::Window() : visibleWindow(NoWindow),previousVisibleWindow(NoWindow),sizeToFit(false) {
    directoryChanged = true;
    isStereoVideo = 0;
    selectedFile = 0;
    selectedVideo = false;
    currentPath = Filesystem::getHomeDirectory();
    selectedCamera = 0;
    selectedCameraID = 0;
    textRenderer = new TextRenderer();
    
    fileTypes.insert("3gp");
    fileTypes.insert("3iv2");
    fileTypes.insert("3ivd");
    fileTypes.insert("a52");
    fileTypes.insert("aac");
    fileTypes.insert("asf");
    fileTypes.insert("asv1");
    fileTypes.insert("asv2");
    fileTypes.insert("au");
    fileTypes.insert("avi");
    fileTypes.insert("bbcd");
    fileTypes.insert("col0");
    fileTypes.insert("col1");
    fileTypes.insert("cvid");
    fileTypes.insert("div1");
    fileTypes.insert("div2");
    fileTypes.insert("div3");
    fileTypes.insert("div4");
    fileTypes.insert("div5");
    fileTypes.insert("div6");
    fileTypes.insert("divx");
    fileTypes.insert("dts");
    fileTypes.insert("dv");
    fileTypes.insert("flac");
    fileTypes.insert("flv");
    fileTypes.insert("fmp4");
    fileTypes.insert("fourcc");
    fileTypes.insert("fsv1");
    fileTypes.insert("h261");
    fileTypes.insert("h262");
    fileTypes.insert("h263");
    fileTypes.insert("hdv1");
    fileTypes.insert("hdv2");
    fileTypes.insert("hdv3");
    fileTypes.insert("iv31");
    fileTypes.insert("iv32");
    fileTypes.insert("iv41");
    fileTypes.insert("iv51");
    fileTypes.insert("m4s2");
    fileTypes.insert("mka");
    fileTypes.insert("mkv");
    fileTypes.insert("mov");
    fileTypes.insert("mp1v");
    fileTypes.insert("mp2");
    fileTypes.insert("mp2v");
    fileTypes.insert("mp3");
    fileTypes.insert("mp4");
    fileTypes.insert("mp41");
    fileTypes.insert("mp42");
    fileTypes.insert("mp4s");
    fileTypes.insert("mp4v");
    fileTypes.insert("mpeg");
    fileTypes.insert("mpg");
    fileTypes.insert("mpg1");
    fileTypes.insert("mpg2");
    fileTypes.insert("mpg3");
    fileTypes.insert("mpg4");
    fileTypes.insert("nsc");
    fileTypes.insert("nsv");
    fileTypes.insert("nut");
    fileTypes.insert("ogg");
    fileTypes.insert("ogm");
    fileTypes.insert("pim1");
    fileTypes.insert("qdrw");
    fileTypes.insert("ra");
    fileTypes.insert("ram");
    fileTypes.insert("rle");
    fileTypes.insert("rm");
    fileTypes.insert("rmbv");
    fileTypes.insert("rpza");
    fileTypes.insert("rv");
    fileTypes.insert("rv10");
    fileTypes.insert("rv13");
    fileTypes.insert("rv20");
    fileTypes.insert("rv30");
    fileTypes.insert("rv40");
    fileTypes.insert("smc");
    fileTypes.insert("smp4");
    fileTypes.insert("svq1");
    fileTypes.insert("svq3");
    fileTypes.insert("tac");
    fileTypes.insert("thra");
    fileTypes.insert("ts");
    fileTypes.insert("tta");
    fileTypes.insert("ty");
    fileTypes.insert("vcr2");
    fileTypes.insert("vid");
    fileTypes.insert("vp3");
    fileTypes.insert("vp30");
    fileTypes.insert("vp31");
    fileTypes.insert("vp5");
    fileTypes.insert("vp50");
    fileTypes.insert("vp51");
    fileTypes.insert("vp60");
    fileTypes.insert("vp61");
    fileTypes.insert("vp62");
    fileTypes.insert("vp6a");
    fileTypes.insert("vp6f");
    fileTypes.insert("vp7");
    fileTypes.insert("wav");
    fileTypes.insert("wmv");
    fileTypes.insert("wmv1");
    fileTypes.insert("wmv2");
    fileTypes.insert("wmv3");
    fileTypes.insert("wmva");
    fileTypes.insert("wvc1");
    fileTypes.insert("xa");
    fileTypes.insert("xvid");
}

void Ibex::Window::renderInfoWindow() {
    updateRender = false;
    std::vector<std::string> lines;
	lines.push_back(" H. Help ");
    lines.push_back(" 1. Load Video ");
    lines.push_back(" 2. Load Stereo Video ");
    lines.push_back(" 3. Camera ");
    lines.push_back(" 4. Stereo Camera ");
    lines.push_back(" ");
    lines.push_back(fpsString);
    textRenderer->renderTextToFramebuffer(0, 0, lines, std::vector<bool>());
}

void Ibex::Window::renderSettingChangeMessage() {
    updateRender = false;
    fade = 2.0;
    std::vector<std::string> lines;
    lines.push_back(settingsChangedMessage);
    textRenderer->renderTextToFramebuffer(0, 0, lines, std::vector<bool>());
}
void Ibex::Window::renderHelpWindow() {
    updateRender = false;
    std::vector<std::string> lines;
    lines.push_back(" Help: Backpace to go back ");
    lines.push_back(" / - toggle dialog (including help) ");
    lines.push_back(" W/S - forward/back   A/D - left/right ");
    lines.push_back(" Shift - run          R - reset ");
#ifdef WIN32
	lines.push_back(" Cntrl+Shift+G - toggle control desktop ");
#else
    lines.push_back(" Fn+Shift+F1 - toggle control desktop ");
#endif
    lines.push_back(" \\ - bring up desktop where looking ");
#ifdef __APPLE__
    lines.push_back(" Fn+Shift+F2 - lower rendering quality ");
#endif
    lines.push_back(" L - lock head-tracking   U - walk follows view ");
    lines.push_back(" U - toggle walking follows view ");
    textRenderer->renderTextToFramebuffer(0, 0, lines, std::vector<bool>());
}

void Ibex::Window::renderFileChooser() {
    updateRender = false;
#ifdef _WIN32
	uint listingOffset = 1;
#else
	uint listingOffset = 2;
#endif
    if(directoryChanged) {
        directoryList = Filesystem::listDirectory(currentPath.c_str());
        uint count = 0;
		if(directoryList.size()) {
			for(auto i = directoryList.end()-1; i >= (directoryList.begin()+listingOffset);++count) {
				if(*i != "..") {
					bool found = false;
                    const unsigned long len = (*i).length();
                    const std::string last3 = (len >= 3) ? (*i).substr(len-3,3) : "";
                    const std::string last4 = (len >= 4) ? (*i).substr(len,4) : "";
                    if(fileTypes.find(last3)!=fileTypes.end() || fileTypes.find(last4)!=fileTypes.end()) found = true;
                    
					if((*i).size() && (*i)[0] == '*') {
						if((*i).size() > 1 && (*i)[1] == '.') {
							directoryList.erase(i--);
							continue;
						}
						found = true;
					}
					if(!found) {
						directoryList.erase(i--);
						std::cerr << directoryList.size() << std::endl;
						continue;
					}
				}
				--i;
			}
			std::sort(directoryList.begin()+((directoryList.size() >= 2) ? 2 : 1), directoryList.end(), [](const std::string &a, const std::string &b){
				bool aa = (a.size()) ? a[0] == '*' : 0;
				bool bb = (b.size()) ? b[0] == '*' : 0;
				if(aa == bb) {
					return a < b;
				}
				return bb;
			});
		}
        directoryChanged = false;
    }
    
    uint startIndex = (selectedFile > NUM_LINES/2) ? selectedFile-NUM_LINES/2 : 0;
    uint endIndex = fmin(startIndex+NUM_LINES-1,directoryList.size());
    std::vector<bool> highlighted;
    for(int i = 0; i < endIndex-startIndex; ++i) {
        highlighted.push_back(i == ((selectedFile > NUM_LINES/2)?(NUM_LINES/2):selectedFile));
    }
    textRenderer->renderTextToFramebuffer(0, 0, std::vector<std::string>(directoryList.begin()+startIndex, directoryList.begin()+endIndex), highlighted, 30);
}

void Ibex::Window::renderCameraChooser() {
    updateRender = false;
    std::vector<std::string> lines;
    lines.push_back("~/Backspace: Back ");
    uint startIndex = (selectedFile > NUM_LINES/2) ? selectedFile-NUM_LINES/2 : 0;
    for(uint i = startIndex,index = 0; i < startIndex+NUM_LINES && i < cameras.size(); ++i,++index) {
        std::stringstream ss;
        ss << i+1 << ". Camera " << cameras[i] << " ";
        lines.push_back(ss.str());
    }
    uint endIndex = fmin(startIndex+NUM_LINES-1,cameras.size());
    std::vector<bool> highlighted;
    for(int i = 0; i < endIndex-startIndex+1; ++i) {
        highlighted.push_back((i-1) == ((selectedFile > NUM_LINES/2)?(NUM_LINES/2):selectedFile));
    }
    textRenderer->renderTextToFramebuffer(0, 0, lines, highlighted);
}

void Ibex::Window::reset() {
	directoryChanged = true;
	directoryList.clear();
	selectedFile = 0;
	selectedCamera = 0;
	isStereoVideo = 0;
	selectedCameraID = 0;
	visibleWindow = InfoWindow;
}

bool Ibex::Window::toggleShowDialog() {
    if(visibleWindow == SettingChangeMessage) ::showDialog = true;
    else ::showDialog = !::showDialog;
    return ::showDialog;
}

void Ibex::Window::changedSettingMessage(const std::string &message) {
    updateRender = true;
    settingsChangedMessage = message;
    visibleWindow = SettingChangeMessage;
    ::showDialog = true;
}

void Ibex::Window::update(const double &timeDelta) {
    bool update = false;
    static double time = 0;
    if(time > 5.0) {
        if(visibleWindow == InfoWindow) {
            update = true;
        }
        time = fmod(time, 5.0);
    }
    time += timeDelta;
    if(previousVisibleWindow != visibleWindow || updateRender) {
        previousVisibleWindow = visibleWindow;
        update = true;
    }
    
    if(::showDialog && update) {
        switch(visibleWindow) {
            case FileChooser:
                renderFileChooser();
                break;
            case CameraChooser:
                renderCameraChooser();
                break;
            case HelpWindow:
                renderHelpWindow();
                break;
            case SettingChangeMessage:
                renderSettingChangeMessage();
                break;
            case InfoWindow:
            default:
                renderInfoWindow();
                break;
        }
    }
}
void Ibex::Window::render(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP, const double &timeDelta) {
    if(visibleWindow != NoWindow) {
        double fadeLevel = (fade > 0.5) ? 1 : fade*2;
        if(visibleWindow == SettingChangeMessage && fade < 0) {
            ::showDialog = false;
            visibleWindow = NoWindow;
            return;
        }
		const bool sizeToFit = (visibleWindow == SettingChangeMessage);
        textRenderer->renderText(MVP, V, M, shadowPass, depthMVP, (visibleWindow == SettingChangeMessage) ? fadeLevel : 1.0, sizeToFit);
        fade -= timeDelta;
    }
}

#ifdef __APPLE__
int Ibex::Window::processKey(unsigned short keyCode, int down) {
    int processed = 0;

	if(visibleWindow == SettingChangeMessage) {
		switch(keyCode) {
			case kVK_Escape:
			if(!down) {
				::showDialog = false;
				visibleWindow = NoWindow;
				updateRender = true;
			}
            
            processed = 1;
		}
		if(processed) updateRender = true;

		return processed;
	}

    switch(keyCode) {
        case kVK_UpArrow:
        case kVK_ANSI_W:
            if(down) {
                --selectedFile;
                if(visibleWindow == InfoWindow) {
                    if(directoryList.size() < 1)selectedFile = 0;
                    else if(selectedFile >= directoryList.size()) selectedFile = directoryList.size()-1;
                } else if(visibleWindow == CameraChooser) {
                    if(cameras.size() < 1)selectedFile = 0;
                    else if(selectedFile >= cameras.size()) selectedFile = cameras.size()-1;
                }
                updateRender = true;
            }
            processed = 1;
            break;
        case kVK_DownArrow:
        case kVK_ANSI_S:
            if(down) {
                ++selectedFile;
                if(visibleWindow == InfoWindow) {
                    if(directoryList.size() < 1)selectedFile = 0;
                    else selectedFile %= directoryList.size();
                } else if(visibleWindow == CameraChooser) {
                    if(cameras.size() < 1)selectedFile = 0;
                    else selectedFile %= cameras.size();
                }
                updateRender = true;
            }
            
            processed = 1;
            break;
        case kVK_Delete:
            visibleWindow = InfoWindow;
            
            processed = 1;
            break;
        case kVK_Return:
            if(down) {
                switch(visibleWindow) {
                    case FileChooser:
                    {
                        if(selectedFile < directoryList.size()) {
                            std::string fullPath = Filesystem::getFullPath(currentPath, directoryList[selectedFile]);
                            if(Filesystem::isFile(fullPath) && !Filesystem::isDirectory(fullPath)) {
                                selectedVideo = true;
                                videoPath = fullPath;
                                visibleWindow = NoWindow;
                                ::showDialog = false;
                            } else {
                                currentPath = Filesystem::navigate(currentPath, directoryList[selectedFile]);
                            }
                            directoryChanged = true;
                            selectedFile = 0;
                        }
                        break;
                    }
                    case CameraChooser:
                    {
                        if(selectedFile < cameras.size()) {
                            selectedCamera = true;
                            selectedCameraID = cameras[selectedFile];
                            visibleWindow = NoWindow;
                            ::showDialog = false;
                            selectedFile = 0;
                        } else {
                            
                        }
                        break;
                    }
                    default:
                        break;
                }
                updateRender = true;
//            showDialog = false;
            }
            
            processed = 1;
            break;
        case kVK_ANSI_H:
            if(down) {
                if(::showDialog && visibleWindow == HelpWindow) {
                    ::showDialog = false;
                    visibleWindow = NoWindow;
                    updateRender = true;
                }
                if(visibleWindow == InfoWindow) {
                    visibleWindow = HelpWindow;
                }
            }
            
            processed = 1;
            break;
        case kVK_ANSI_2:
        case kVK_ANSI_1:
            if(down) {
                if(visibleWindow != FileChooser) {
                    directoryList.clear();
                    selectedFile = 0;
                    isStereoVideo = (keyCode == kVK_ANSI_2);
                    directoryChanged = true;
                }
                if(visibleWindow == InfoWindow) {
                    visibleWindow = FileChooser;
                }
                updateRender = true;
            }
            
            processed = 1;
            break;
        case kVK_ANSI_4:
        case kVK_ANSI_3:
            if(down) {
                if(visibleWindow != FileChooser) {
                    directoryList.clear();
                    selectedCamera = 0;
                    selectedCameraID = -1;
                    isStereoVideo = (keyCode == kVK_ANSI_4);
                    directoryChanged = true;
                }
                if(visibleWindow == InfoWindow) {
                    visibleWindow = CameraChooser;
                    cameras = VLCVideoPlayer::listCameras();
                }
                updateRender = true;
            }
            
            processed = 1;
            break;
        case kVK_Escape:
            ::showDialog = false;
            visibleWindow = NoWindow;
            updateRender = true;
            
            processed = 1;
            break;
    }
    return processed;
}
#else
#ifdef _WIN32
int Ibex::Window::processKey(int key, int down) {
	int processed = 0;
	if(visibleWindow == SettingChangeMessage) {
		switch(key) {
			case GLFW_KEY_ESCAPE:
			if(!down) {
				::showDialog = false;
				visibleWindow = NoWindow;
				updateRender = true;
			}
            
            processed = 1;
		}
		if(processed) updateRender = true;

		return processed;
	}
    switch(key) {
		case 'W':
        case 'w':
		case GLFW_KEY_UP:
            if(down) {
                --selectedFile;
                if(visibleWindow == InfoWindow) {
                    if(directoryList.size() < 1)selectedFile = 0;
                    else if(selectedFile >= directoryList.size()) selectedFile = directoryList.size()-1;
                } else if(visibleWindow == CameraChooser) {
                    if(cameras.size() < 1)selectedFile = 0;
                    else if(selectedFile >= cameras.size()) selectedFile = cameras.size()-1;
                }
            }
            processed = 1;
            break;
		case GLFW_KEY_DOWN:
		case 'S':
		case 's':
            if(down) {
                ++selectedFile;
                if(visibleWindow == InfoWindow) {
                    if(directoryList.size() < 1)selectedFile = 0;
                    else selectedFile %= directoryList.size();
                } else if(visibleWindow == CameraChooser) {
                    if(cameras.size() < 1)selectedFile = 0;
                    else selectedFile %= cameras.size();
                }
            }
            
            processed = 1;
            break;
        case 'h':
        case 'H':
            if(!down) {
                if(::showDialog && visibleWindow == HelpWindow) {
                    ::showDialog = false;
                    visibleWindow = NoWindow;
                    updateRender = true;
                }
                if(visibleWindow == InfoWindow) {
                    visibleWindow = HelpWindow;
                }
            }
            
            processed = 1;
            break;
        case '1':
        case '2':
            if(!down) {
                if(visibleWindow == InfoWindow) {
					reset();
                    directoryList.clear();
                    selectedFile = 0;
                    isStereoVideo = (key == '2');
                    directoryChanged = true;
                }
                if(visibleWindow == InfoWindow) {
                    visibleWindow = FileChooser;
                }
            }
            
            processed = 1;
            break;
        case '3':
		case '4':
            if(!down) {
                if(visibleWindow == InfoWindow) {
					reset();
                    directoryList.clear();
                    selectedCamera = 0;
                    selectedCameraID = -1;
                    isStereoVideo = (key == '4');
                    directoryChanged = true;
                }
                if(visibleWindow == InfoWindow) {
                    visibleWindow = CameraChooser;
                    cameras = VLCVideoPlayer::listCameras();
                }
                updateRender = true;
            }
            
            processed = 1;
            break;
		// case 8: // BACKSPACE
        // case 127: // DELETE
		case GLFW_KEY_BACKSPACE:
		case GLFW_KEY_DELETE:
			if(!down) {
				visibleWindow = InfoWindow;
			}
            processed = 1;
            break;
        // case 13: // ENTER KEY
		case GLFW_KEY_ENTER:
			if(!down) {
                switch(visibleWindow) {
                    case FileChooser:
                    {
                        if(selectedFile < directoryList.size() && selectedFile >= 0) {
                            std::string fullPath = Filesystem::getFullPath(currentPath, directoryList[selectedFile]);
                            if(Filesystem::isFile(fullPath) && !Filesystem::isDirectory(fullPath)) {
                                selectedVideo = true;
                                videoPath = fullPath;
                                visibleWindow = NoWindow;
                                ::showDialog = false;
                            } else {
                                currentPath = Filesystem::navigate(currentPath, directoryList[selectedFile]);
                            }
                            directoryChanged = true;
							directoryList.clear();
                            selectedFile = 0;
                        }
                        break;
                    }
                    case CameraChooser:
                    {
                        if(selectedFile >= 0 && selectedFile < cameras.size()) {
							directoryList.clear();
                            selectedCamera = true;
                            selectedCameraID = cameras[selectedFile];
                            visibleWindow = NoWindow;
                            ::showDialog = false;
                            selectedFile = 0;
                        } else {
                            
                        }
                        break;
                    }
                    default:
                        break;
                }
//            showDialog = false;
            }
            
            processed = 1;
            break;
        // case 27: // ESCAPE
		case GLFW_KEY_ESCAPE:
			if(!down) {
				::showDialog = false;
				visibleWindow = NoWindow;
				updateRender = true;
			}
            
            processed = 1;
    }
	if(processed) updateRender = true;

    return processed;
}
int Ibex::Window::processSpecialKey(int key, int down) {
	int processed = 0;
	// special keys processed as regular keys now that using GLFW not GLUT
    return processed;
}
#else
int Ibex::Window::processKey(XIDeviceEvent *event, bool down) {
  static KeyCode B = XKeysymToKeycode(dpy, XK_B); // toggle barrel distort
  static KeyCode G = XKeysymToKeycode(dpy, XK_G); // toggle ground

  static KeyCode KC1 = XKeysymToKeycode(dpy, XK_1);
  static KeyCode KC2 = XKeysymToKeycode(dpy, XK_2);
  static KeyCode KC3 = XKeysymToKeycode(dpy, XK_3);
  static KeyCode KC4 = XKeysymToKeycode(dpy, XK_4);
  
  static KeyCode W = XKeysymToKeycode(dpy, XK_W);
  static KeyCode S = XKeysymToKeycode(dpy, XK_S);
  static KeyCode A = XKeysymToKeycode(dpy, XK_A);
  static KeyCode D = XKeysymToKeycode(dpy, XK_D);
  static KeyCode Q = XKeysymToKeycode(dpy, XK_Q);
  static KeyCode E = XKeysymToKeycode(dpy, XK_E);
  static KeyCode R = XKeysymToKeycode(dpy, XK_R);
  static KeyCode FORWARD_SLASH = XKeysymToKeycode(dpy, XK_slash);
  static KeyCode SPACE = XKeysymToKeycode(dpy, XK_space);
  static KeyCode ENTER = XKeysymToKeycode(dpy, XK_Return);

  KeyCode key = event->detail;
  int processed = 0;
  if(key == W) {
    if(down) {
      --selectedFile;
      if(selectedFile < 0 && directoryList.size() > 0) selectedFile += directoryList.size();
      else if(directoryList.size() <= 0 && selectedFile < 0)selectedFile = 0;
    }
    processed = 1;
  } else if(key == S) {
    if(down) {
      ++selectedFile;
      if(directoryList.size() > 0) {
	selectedFile %= directoryList.size();
      }
    }
    processed = 1;
  } else if(key == ENTER) {
    if(down) {
      switch(visibleWindow) {
      case FileChooser:
	{
	  if(selectedFile < directoryList.size() && selectedFile >= 0) {
	    std::string fullPath = Filesystem::getFullPath(currentPath, directoryList[selectedFile]);
	    if(Filesystem::isFile(fullPath) && !Filesystem::isDirectory(fullPath)) {
	      selectedVideo = true;
	      videoPath = fullPath;
	      showDialog = false;
	    } else {
	      currentPath = Filesystem::navigate(currentPath, directoryList[selectedFile]);
	    }
	    directoryChanged = true;
	    directoryList.clear();
	    selectedFile = 0;
	  }
	  break;
	}
      case CameraChooser:
	{
	  if(selectedFile >= 0 && selectedFile < cameras.size()) {
	    directoryList.clear();
	    selectedCamera = true;
	    selectedCameraID = cameras[selectedFile];
	    showDialog = false;
	    selectedFile = 0;
	  } else {
                            
	  }
	  break;
	}
      default:
	break;
      }
      //            showDialog = false;
    }
    processed = 1;
  } else if(key == KC1 || key == KC2) {
    if(down) {
      if(visibleWindow == InfoWindow) {
	reset();
	directoryList.clear();
	selectedFile = 0;
	isStereoVideo = (key == KC2);
	directoryChanged = true;
      }
      visibleWindow = FileChooser;
    }
    
    processed = 1;
  } else if(key == KC3 || key == KC4) {
    /*
    if(down) {
      if(visibleWindow == InfoWindow) {
	reset();
	directoryList.clear();
	selectedCamera = 0;
	selectedCameraID = -1;
	isStereoVideo = (key == KC4);
	directoryChanged = true;
      }
      visibleWindow = CameraChooser;
      cameras = VLCVideoPlayer::listCameras();
    }
    */
    processed = 1;
  }

  return processed;
}
#endif
#endif
