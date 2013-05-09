//
//  Window.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 5/7/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "Window.h"
#include "opengl_helpers.h"

#include "ibex.h"
#include "Filesystem.h"
#include <algorithm>

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
    //    renderStrokeFontString(0, 0.5, -0.25, GLUT_STROKE_ROMAN, fpsString);
    renderBitmapString(0.005, 0.465, -0.25, GLUT_BITMAP_HELVETICA_18, fpsString);
    glEnable(GL_DEPTH_TEST);
}

void Ibex::Window::renderFileChooser() {
    if(directoryChanged) {
        directoryList = Filesystem::listDirectory(currentPath.c_str());
        int count = 0;
        for(auto i = directoryList.end()-1; i >= (directoryList.begin()+2); --i,++count) {
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
                        directoryList.erase(i);
                        continue;
                    }
                    found = true;
                }
                if(!found) {
                    directoryList.erase(i);
                    std::cerr << directoryList.size() << std::endl;
                }
            }
        }
        std::sort(directoryList.begin()+((directoryList.size() >= 2) ? 2 : 1), directoryList.end(), [](const std::string &a, const std::string &b){
            bool aa = (a.size()) ? a[0] == '*' : 0;
            bool bb = (b.size()) ? b[0] == '*' : 0;
            if(aa == bb) {
                return a < b;
            }
            return bb;
        });
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

void Ibex::Window::render() {
    switch(visibleWindow) {
        case FileChooser:
            renderFileChooser();
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
                if(selectedFile < 0) selectedFile += directoryList.size();
            }
            processed = 1;
            break;
        case kVK_DownArrow:
        case kVK_ANSI_S:
            if(down) {
                ++selectedFile;
                selectedFile %= directoryList.size();
            }
            
            processed = 1;
            break;
        case kVK_Delete:
            visibleWindow = InfoWindow;
            
            processed = 1;
            break;
        case kVK_Return:
            if(down) {
                if(selectedFile < directoryList.size()) {
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
//            showDialog = false;
            }
            
            processed = 1;
            break;
        case kVK_ANSI_1:
            if(down) {
                if(visibleWindow != FileChooser) {
                    selectedFile = 0;
                    isStereoVideo = false;
                    directoryChanged = true;
                }
                visibleWindow = FileChooser;
            }
            
            processed = 1;
            break;
        case kVK_ANSI_2:
            if(down) {
                if(visibleWindow != FileChooser) {
                    selectedFile = 0;
                    isStereoVideo = true;
                    directoryChanged = true;
                }
                visibleWindow = FileChooser;
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
#endif
