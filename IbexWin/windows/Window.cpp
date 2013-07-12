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
#include <string>
#include "../video/VideoPlayer.h"

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

bool endsWith(std::string const &inputString, std::string const &ending)
{
    if (inputString.length() >= ending.length())
        return (inputString.compare(inputString.length() - ending.length(), ending.length(), ending) == 0);
    else
        return false;
}

Ibex::Window::Window() : visibleWindow(InfoWindow) {
    directoryChanged = true;
    isStereoVideo = 0;
    selectedFile = 0;
    selectedVideo = false;
    currentPath = Filesystem::getHomeDirectory();
    selectedCamera = 0;
    selectedCameraID = 0;
    
    fileTypes.insert("avi");
    fileTypes.insert("mov");
    fileTypes.insert(".qt");
    fileTypes.insert("mp4");
    fileTypes.insert("mpg");
}
void renderBitmapString(
                        float x,
                        float y,
                        float z,
                        void *font,
                        char *string) {
    
    char *c;
    
    glPushMatrix();
    glRasterPos3f(x, y,z);
    glScaled(0.001/2.0, 0.001/2.0, 0.001/2.0);
    for (c=string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
    glPopMatrix();
}

void renderStrokeFontString(
                            float x,
                            float y,
                            float z,
                            void *font,
                            char *string) {
    
    char *c;
    glPushMatrix();
    glTranslatef(x, y,z);
    
    glScaled(0.001/20.0, 0.001/20.0, 0.001/20.0);
    for (c=string; *c != '\0'; c++) {
        glutStrokeCharacter(font, *c);
    }
    
    glPopMatrix();
}

void Ibex::Window::renderInfoWindow() {
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_DEPTH_TEST);
    glColor4f(0,0.1,0,0.5);
    glBegin(GL_QUADS);
    glVertex3d(-0.05, 0.45, -0.25);
    glVertex3d(0.05, 0.45, -0.25);
    glVertex3d(0.05, 0.55, -0.25);
    glVertex3d(-0.05, 0.55, -0.25);
    glEnd();
    glColor4f(1,1,1,1);
    renderBitmapString(-0.045, 0.54, -0.25, GLUT_BITMAP_HELVETICA_18, "1. Load Video");
    renderBitmapString(-0.045, 0.53, -0.25, GLUT_BITMAP_HELVETICA_18, "2. Load Stereo Video");
    renderBitmapString(-0.045, 0.52, -0.25, GLUT_BITMAP_HELVETICA_18, "3. Camera");
    renderBitmapString(-0.045, 0.51, -0.25, GLUT_BITMAP_HELVETICA_18, "4. Stereo Camera");
    //    renderStrokeFontString(0, 0.5, -0.25, GLUT_STROKE_ROMAN, fpsString);
    renderBitmapString(0.005, 0.465, -0.25, GLUT_BITMAP_HELVETICA_18, fpsString);
    glEnable(GL_DEPTH_TEST);
}

void Ibex::Window::renderFileChooser() {
#ifdef _WIN32
	int listingOffset = 1;
#else
	int listingOffset = 2;
#endif
    if(directoryChanged) {
        directoryList = Filesystem::listDirectory(currentPath.c_str());
        int count = 0;
		if(directoryList.size()) {
			for(auto i = directoryList.end()-1; i >= (directoryList.begin()+listingOffset);++count) {
				if(*i != "..") {
					bool found = false;
					for(auto i2 = fileTypes.begin(); i2!= fileTypes.end(); ++i2) {
						if(endsWith((*i), *i2)) {
							found = true;
							break;
						}
					}
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
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_DEPTH_TEST);
    glColor4f(0,0.1,0,0.5);
    glBegin(GL_QUADS);
    glVertex3d(-0.1, 0.35, -0.25);
    glVertex3d(0.1, 0.35, -0.25);
    glVertex3d(0.1, 0.65, -0.25);
    glVertex3d(-0.1, 0.65, -0.25);
    glEnd();
    glColor4f(1,1,1,1);
    //    renderStrokeFontString(0, 0.5, -0.25, GLUT_STROKE_ROMAN, fpsString);
    renderBitmapString(-0.095, 0.64, -0.25, GLUT_BITMAP_HELVETICA_18, "~/Backspace: Back");
    //    renderBitmapString(-0.045, 0.53, -0.25, GLUT_BITMAP_HELVETICA_18, "2. ");
    char blah[256];
    int startIndex = (selectedFile > 28/2) ? selectedFile-28/2 : 0;
    for(int i = startIndex,index = 0; i < startIndex+28 && i < directoryList.size(); ++i,++index) {
        std::string pathWithoutDir = directoryList[i];
        if(directoryList[i].size() && directoryList[i][0] == '*') {
            pathWithoutDir = pathWithoutDir.substr(1);
        }
        
        sprintf(blah,"%d. %s",i+1,(i < directoryList.size()) ? pathWithoutDir.c_str() : "---------");
        
        if(directoryList[i].size() && directoryList[i][0] == '*') {
            glColor4f(0.,0.,1,1);
        }
        if(selectedFile == i) {
            glColor4f(1,1,0,1);
        }
        renderBitmapString(-0.095, 0.63-index*0.01, -0.25, GLUT_BITMAP_HELVETICA_18, blah);
        glColor4f(1,1,1,1);
    }
    renderBitmapString(0.055, 0.36, -0.25, GLUT_BITMAP_HELVETICA_18, fpsString);
    glEnable(GL_DEPTH_TEST);
}

void Ibex::Window::renderCameraChooser() {
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_DEPTH_TEST);
    glColor4f(0,0.1,0,0.5);
    glBegin(GL_QUADS);
    glVertex3d(-0.1, 0.35, -0.25);
    glVertex3d(0.1, 0.35, -0.25);
    glVertex3d(0.1, 0.65, -0.25);
    glVertex3d(-0.1, 0.65, -0.25);
    glEnd();
    glColor4f(1,1,1,1);
    //    renderStrokeFontString(0, 0.5, -0.25, GLUT_STROKE_ROMAN, fpsString);
    renderBitmapString(-0.095, 0.64, -0.25, GLUT_BITMAP_HELVETICA_18, "~/Backspace: Back");
    //    renderBitmapString(-0.045, 0.53, -0.25, GLUT_BITMAP_HELVETICA_18, "2. ");
    char blah[256];
    int startIndex = (selectedFile > 28/2) ? selectedFile-28/2 : 0;
    for(int i = startIndex,index = 0; i < startIndex+28 && i < cameras.size(); ++i,++index) {
        sprintf(blah,"%d. Camera %d",i+1, cameras[i]);
        
        if(selectedFile == i) {
            glColor4f(1,1,0,1);
        }
        renderBitmapString(-0.095, 0.63-index*0.01, -0.25, GLUT_BITMAP_HELVETICA_18, blah);
        glColor4f(1,1,1,1);
    }
    renderBitmapString(0.055, 0.36, -0.25, GLUT_BITMAP_HELVETICA_18, fpsString);
    glEnable(GL_DEPTH_TEST);
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

void Ibex::Window::render() {
    switch(visibleWindow) {
        case FileChooser:
            renderFileChooser();
            break;
        case CameraChooser:
            renderCameraChooser();
            break;
        case InfoWindow:
        default:
            renderInfoWindow();
            break;
    }
}

#ifdef __APPLE__
int Ibex::Window::processKey(unsigned short keyCode, int down) {
    int processed = 0;
    switch(keyCode) {
        case kVK_UpArrow:
        case kVK_ANSI_W:
            if(down) {
                --selectedFile;
                if(selectedFile < 0 && directoryList.size()) selectedFile += directoryList.size();
                else if(!directoryList.size())selectedFile = 0;
            }
            processed = 1;
            break;
        case kVK_DownArrow:
        case kVK_ANSI_S:
            if(down) {
                ++selectedFile;
                if(directoryList.size()) {
                    selectedFile %= directoryList.size();
                }
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
                            selectedFile = 0;
                        }
                        break;
                    }
                    case CameraChooser:
                    {
                        if(selectedFile >= 0 && selectedFile < cameras.size()) {
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
                visibleWindow = FileChooser;
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
                visibleWindow = CameraChooser;
                cameras = VideoPlayer::listCameras();
            }
            
            processed = 1;
            break;
        case kVK_Escape:
            showDialog = false;
            
            processed = 1;
            break;
    }
    return processed;
}
#else
#ifdef _WIN32
int Ibex::Window::processKey(unsigned char key, int down) {
	int processed = 0;
    switch(key) {
		case 'W':
        case 'w':
            if(down) {
                --selectedFile;
                if(selectedFile < 0 && directoryList.size() > 0) selectedFile += directoryList.size();
				if(directoryList.size() <= 0 && selectedFile < 0) selectedFile = 0;
            }
            processed = 1;
            break;
		case 'S':
		case 's':
            if(down) {
                ++selectedFile;
				if(directoryList.size() > 0) {
					selectedFile %= directoryList.size();
				} else {
					//selectedFile = 0;
				}
            }
            
            processed = 1;
            break;
        case '1':
        case '2':
            if(down) {
                if(visibleWindow == InfoWindow) {
					reset();
                    directoryList.clear();
                    selectedFile = 0;
                    isStereoVideo = (key == '2');
                    directoryChanged = true;
                }
                visibleWindow = FileChooser;
            }
            
            processed = 1;
            break;
        case '3':
		case '4':
            if(down) {
                if(visibleWindow == InfoWindow) {
					reset();
                    directoryList.clear();
                    selectedCamera = 0;
                    selectedCameraID = -1;
                    isStereoVideo = (key == '4');
                    directoryChanged = true;
                }
                visibleWindow = CameraChooser;
                cameras = VideoPlayer::listCameras();
            }
            
            processed = 1;
            break;
		case 8: // BACKSPACE
        case 127: // DELETE
            visibleWindow = InfoWindow;
            
            processed = 1;
            break;
        case 13: // ENTER KEY
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
            break;
        case 27: // ESCAPE
            showDialog = false;
            
            processed = 1;
            break;
    }
    return processed;
}
int Ibex::Window::processSpecialKey(unsigned char key, int down) {
	int processed = 0;
    switch(key) {
        case GLUT_KEY_UP:
            if(down) {
                --selectedFile;
                if(selectedFile < 0 && directoryList.size() > 0) selectedFile += directoryList.size();
                else if(directoryList.size() <= 0 && selectedFile < 0)selectedFile = 0;
            }
            processed = 1;
            break;
        case GLUT_KEY_DOWN:
            if(down) {
                ++selectedFile;
                if(directoryList.size() > 0) {
                    selectedFile %= directoryList.size();
                }
            }

            processed = 1;
            break;
    }
    return processed;
}
#endif
#endif
