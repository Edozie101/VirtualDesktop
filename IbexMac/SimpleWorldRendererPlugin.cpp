/*
 * SimpleWorldRendererPlugin.cpp
 *
 *  Created on: Sep 25, 2012
 *      Author: Hesham Wahba
 */

#include <iostream>

#include "distortions.h"
#include "opengl_helpers.h"
#include "iphone_orientation_plugin/iphone_orientation_listener.h"

#include "ibex_mac_utils.h"

#include "SimpleWorldRendererPlugin.h"

SimpleWorldRendererPlugin::SimpleWorldRendererPlugin() {
}
SimpleWorldRendererPlugin::~SimpleWorldRendererPlugin() {
}

// ---------------------------------------------------------------------------
// Function: loadSkybox
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void SimpleWorldRendererPlugin::loadSkybox()
{
    _skybox[0] = loadTexture("/resources/humus-skybox/negz.jpg");
    _skybox[1] = loadTexture("/resources/humus-skybox/posx.jpg");
    _skybox[2] = loadTexture("/resources/humus-skybox/posz.jpg");
    _skybox[3] = loadTexture("/resources/humus-skybox/negx.jpg");
    _skybox[4] = loadTexture("/resources/humus-skybox/posy.jpg");
    _skybox[5] = loadTexture("/resources/humus-skybox/negy.jpg");
}


// ---------------------------------------------------------------------------
// Function: renderSkybox
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void SimpleWorldRendererPlugin::renderSkybox()
{
  static const double skyboxScale = 1000;
  // Store the current matrix
  glPushMatrix();
  glScaled(skyboxScale, skyboxScale, skyboxScale);

  // Enable/Disable features
  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);
    
  // Just in case we set all vertices to white.
  glColor4f(1, 1, 1, 1);

  // Render the front quad
//    checkForErrors();
//    std::cerr << "bind texture... " << _skybox[1] << std::endl;
  glBindTexture(GL_TEXTURE_2D, _skybox[0]);
//    checkForErrors();
//        std::cerr << "bind texture... done" << std::endl;
  glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
    glTexCoord2f(1, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
    glTexCoord2f(1, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
    glTexCoord2f(0, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
  glEnd();
    
  // Render the left quad
  glBindTexture(GL_TEXTURE_2D, _skybox[1]);
  glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(  0.5f, -0.5f,  0.5f );
    glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
    glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
    glTexCoord2f(0, 1); glVertex3f(  0.5f,  0.5f,  0.5f );
  glEnd();

  // Render the back quad
  glBindTexture(GL_TEXTURE_2D, _skybox[2]);
  glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f,  0.5f );
    glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f,  0.5f );
    glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f,  0.5f );
    glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f,  0.5f );
  glEnd();

  // Render the right quad
  glBindTexture(GL_TEXTURE_2D, _skybox[3]);
  glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
    glTexCoord2f(1, 0); glVertex3f( -0.5f, -0.5f,  0.5f );
    glTexCoord2f(1, 1); glVertex3f( -0.5f,  0.5f,  0.5f );
    glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
  glEnd();

  // Render the top quad
  glBindTexture(GL_TEXTURE_2D, _skybox[4]);
  glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f( -0.5f,  0.5f, -0.5f );
    glTexCoord2f(0, 0); glVertex3f( -0.5f,  0.5f,  0.5f );
    glTexCoord2f(1, 0); glVertex3f(  0.5f,  0.5f,  0.5f );
    glTexCoord2f(1, 1); glVertex3f(  0.5f,  0.5f, -0.5f );
  glEnd();

  // Render the bottom quad
  glBindTexture(GL_TEXTURE_2D, _skybox[5]);
  glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f( -0.5f, -0.5f, -0.5f );
    glTexCoord2f(0, 1); glVertex3f( -0.5f, -0.5f,  0.5f );
    glTexCoord2f(1, 1); glVertex3f(  0.5f, -0.5f,  0.5f );
    glTexCoord2f(1, 0); glVertex3f(  0.5f, -0.5f, -0.5f );
  glEnd();

  // Restore enable bits and matrix
  glPopAttrib();
  glPopMatrix();

//  glBindTexture(GL_TEXTURE_2D, 0);
}

void SimpleWorldRendererPlugin::init() {
  loadSkybox();
}

void SimpleWorldRendererPlugin::step(const Desktop3DLocation &loc, double timeDiff_) {
  static const GLuint groundTexture = loadTexture("/resources/humus-skybox/negy.jpg");
    
  for (int i2 = 0; i2 < 2; ++i2) {
      checkForErrors();
    if (USE_FBO) {
      glBindFramebuffer(GL_FRAMEBUFFER, fbos[i2]);
      if (!checkForErrors()) {
        std::cerr << "GL ISSUE" << std::endl;
        exit(EXIT_FAILURE);
      }
      glPushMatrix();
    } else {
      if (i2 > 0)
        break;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glPushMatrix();
    {
      glTranslated((i2 == 0) ? -0.01 : 0.01, 0, 0);
      glPushMatrix();
      {
        double orientation[16];
        gluInvertMatrix(get_orientation(), orientation);
        glMultMatrixd(orientation);
        glRotated(loc.getXRotation(), 1, 0, 0);
        glRotated(loc.getYRotation(), 0, 1, 0);
//        glTranslated(0, -1.5, 0);
          glTranslated(0, -0.5, 0);

        glPushMatrix();
        {
          renderSkybox();
            glColor4f(1,1,1,1);
            
          glTranslated(loc.getXPosition(),
                                   loc.getYPosition(),
                                   loc.getZPosition());

          glPushMatrix();
          {
            if(showGround) {
              static const int gridSize = 25;
              static const int textureRepeat = 2*gridSize;

              glBindTexture(GL_TEXTURE_2D, groundTexture);
              glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2d(0, 0);
                glVertex3f(-gridSize, 0, -gridSize);

                glTexCoord2d(textureRepeat, 0);
                glVertex3f(gridSize, 0, -gridSize);

                glTexCoord2d(0, textureRepeat);
                glVertex3f(-gridSize, 0, gridSize);

                glTexCoord2d(textureRepeat, textureRepeat);
                glVertex3f(gridSize, 0, gridSize);
              glEnd();
              glBindTexture(GL_TEXTURE_2D, 0);
            }

            if (renderToTexture) {
                  double ySize = ((double)height / (double)width) / 2.0;
                  glTranslated(0, 1.5, 0);
                  const double monitorOriginZ = -0.5;
                glBindTexture(GL_TEXTURE_2D, desktopTexture);
                glColor4f(1,1,1,1);
                  glBegin(GL_TRIANGLE_STRIP);
                    glTexCoord2d(0, 0);
                    glVertex3f(-0.5, -ySize, monitorOriginZ);

                    glTexCoord2d(1, 0);
                    glVertex3f(0.5, -ySize, monitorOriginZ);

                    glTexCoord2d(0, 1);
                    glVertex3f(-0.5, ySize, monitorOriginZ);

                    glTexCoord2d(1, 1);
                    glVertex3f(0.5, ySize, monitorOriginZ);
                  glEnd();
                
                
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                
                glBindTexture( GL_TEXTURE_2D, cursor );
                glBegin(GL_TRIANGLE_STRIP);
                {
                    double x0, x1, y0, y1, z;
                    x0 = -0.5+(cursorPosX/physicalWidth);
                    x1 = -0.5+((cursorPosX+20.0)/physicalWidth);
                    y0 = -ySize+ySize*2*(cursorPosY+0.)/physicalHeight;
                    y1 = -ySize+ySize*2*(cursorPosY-20.)/physicalHeight;
                    
                    z = monitorOriginZ+0.0000001;
                    glTexCoord2d(0, 0);
                    glVertex3f(  x0,  y0, z);
                    
                    glTexCoord2d(1, 0);
                    glVertex3f( x1, y0, z);
                    
                    glTexCoord2d(0, -1);
                    glVertex3f(  x0, y1 ,z);
                    
                    glTexCoord2d(1, -1);
                    glVertex3f(  x1,  y1, z);
                    
                    glTexCoord2d(1, -1);
                    glVertex3f( x1, y1, z);
                    
                    glTexCoord2d(0, -1);
                    glVertex3f(  x0, y1,z);
                }
                glEnd();
                
                  glBindTexture(GL_TEXTURE_2D, 0);
            } else {
                renderDesktopToTexture();
            }
          }
          glPopMatrix();
        }
        glPopMatrix();
      }
      glPopMatrix();
    }
    glPopMatrix();

    if (USE_FBO) {
      glPopMatrix();
    }
  }

  glViewport(0,0, physicalWidth,physicalHeight);
  if (USE_FBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor4f(1, 1, 1, 1);

    if (ortho) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, 1, 0, 1, -1, 1);
      glMatrixMode(GL_MODELVIEW);
    }
      
    if(barrelDistort) {
        render_distorted_frame(true, textures[0]);
        render_distorted_frame(false, textures[1]);
    } else {
      for (int i = 0; i < 2; ++i) {
        if (ortho) {
          double originX = (i == 0) ? 0 : 0.5;
          glBindTexture(GL_TEXTURE_2D, textures[i]);
          glPushMatrix();
          glColor4f(1, 1, 1, 1);
          glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2d(0, bottom);
            glVertex3f(originX, 0, 0);

            glTexCoord2d(1, bottom);
            glVertex3f(originX + 0.5, 0, 0);

            glTexCoord2d(0, top);
            glVertex3f(originX, 1, 0);

            glTexCoord2d(1, top);
            glVertex3f(originX + 0.5, 1, 0);
          glEnd();
          glPopMatrix();
        } else {
          glBindTexture(GL_TEXTURE_2D, textures[i]);
          glPushMatrix();
          glTranslated((i < 1) ? -0.98 : 0, -0.5, -2.4);
          glColor4f(1, 1, 1, 1);
          glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2d(0, bottom);
            glVertex3f(0, 0, 0);

            glTexCoord2d(1, bottom);
            glVertex3f(1,0,0);

            glTexCoord2d(0, top);
            glVertex3f(0,1,0);

            glTexCoord2d(1, top);
            glVertex3f(1,1,0);
          glEnd();
          glPopMatrix();
        }
      }
    }

    if (ortho) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
//      gluPerspective(120.0f, 0.75, 0.01f, 1000.0f);
      gluPerspective(110.0f, 0.81818181, 0.01f, 1000.0f);
      glMatrixMode(GL_MODELVIEW);
    }
    glViewport(0,0, width, height);
  }
}

Window SimpleWorldRendererPlugin::getWindowID() {
    return 0;
}

bool SimpleWorldRendererPlugin::needsSwapBuffers() {
    return true;
}
