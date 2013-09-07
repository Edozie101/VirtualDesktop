/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/

 Copyright (c) 2000-2006 Torus Knot Software Ltd
 Also see acknowledgements in Readme.html

 You may use this sample code for anything you like, it is not covered by the
 LGPL like the rest of the engine.
 -----------------------------------------------------------------------------
 */
/**
 \file
 NativeRenderSystemCommands.h
 \brief
 A sample demonstrating how to add render system specific native code to an existing scene.
 I based this sample on the SkyDome sample, just added a FrameListener
 */
//#define OGRE_CONTAINERS_USE_CUSTOM_MEMORY_ALLOCATOR 0

#ifndef __NativeRenderSystemCommands_h_
#define __NativeRenderSystemCommands_h_

// --- OpenGL ----------------------------------------------------------------
#define GLX_GLXEXT_PROTOTYPES
#ifdef __APPLE__

#include "GL/glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>

#else

#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>
#include <GL/glu.h>

#endif

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>

#include "../ibex.h"

using namespace Ogre;

class NativeRenderSystemCommandsRenderQueueListener : public RenderQueueListener
{
protected:
  virtual void
  NativeRender() = 0;
};

class OpenGLNativeRenderSystemCommandsRenderQueueListener : public NativeRenderSystemCommandsRenderQueueListener
{
protected:
  MovableObject* mObject;
  const Camera* mCamera;
  SceneManager* mSceneMgr;

  void NativeRender()
  {
      static bool init = false;
      if(!init) {
          init = true;
          startDesktopCapture(0, 0);
      }
    const double s = (double)width/(double)height;

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, this->desktopTexture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTranslated(-10, 0, 0);
    glScalef(10.0, 10.0, 10.0);

    glBegin(GL_QUADS);
    // Front face; offset.  Multi-colored, 50% opaque.
    glNormal3f(0.0f, 0.0f, 1.0f);

    glColor4f(0.9f, 0.2f, 0.2f, 0.5f);

    glTexCoord2f(0.f, 0.f);
    glVertex3f(-1.0f*s, -1.0f, 1.3f);

    glColor4f(0.2f, 0.9f, 0.2f, 0.5f);

    glTexCoord2f(1.f, 0.f);
    glVertex3f(1.0f*s, -1.0f, 1.3f);

    glColor4f(0.2f, 0.2f, 0.9f, 0.5f);

    glTexCoord2f(1.f, 1.f);
    glVertex3f(1.0f*s, 1.0f, 1.3f);

    glColor4f(0.1f, 0.1f, 0.1f, 0.5f);

    glTexCoord2f(0.f, 1.f);
    glVertex3f(-1.0f*s, 1.0f, 1.3f);

    glEnd();

  }

public:
  GLuint desktopTexture;
  OpenGLNativeRenderSystemCommandsRenderQueueListener(MovableObject* object,
      const Camera* camera, SceneManager* sceneMgr) :
      mObject(object),
      mCamera(camera),
      mSceneMgr(sceneMgr),
      desktopTexture(0)
  {
  }

  virtual void renderQueueStarted(uint8 queueGroupId, const String& invocation,

  bool& skipThisInvocation)
  {
  }

  virtual void renderQueueEnded(uint8 queueGroupId, const String& invocation, bool& repeatThisInvocation)
  {
    // Set wanted render queue here - make sure there are - make sure that something is on
    // this queue - else you will never pass this if.
    if (queueGroupId != RENDER_QUEUE_MAIN)
      return;

    // save matrices
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity(); //Texture addressing should start out as direct.
    RenderSystem* renderSystem =
        mObject->_getManager()->getDestinationRenderSystem();
    Node* parentNode = mObject->getParentNode();
    renderSystem->_setWorldMatrix(parentNode->_getFullTransform());
    renderSystem->_setViewMatrix(mCamera->getViewMatrix());
    renderSystem->_setProjectionMatrix(mCamera->getProjectionMatrixRS());
    static Pass* clearPass = NULL;
    if (!clearPass)
      {
        MaterialPtr clearMat = MaterialManager::getSingleton().getByName(
                                                                         "BaseWhite");//.staticCast<Material>();
        clearPass = clearMat->getTechnique(0)->getPass(0);
      }
    //Set a clear pass to give the renderer a clear renderstate
    mSceneMgr->_setPass(clearPass, true, false);
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);
    GLboolean stencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
    glDisable(GL_STENCIL_TEST);
    // save attribs
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    // call native rendering function
    //////////////////
    NativeRender();
    //////////////////
    // restore original state
    glPopAttrib();
    if (depthTestEnabled) {
      glEnable(GL_DEPTH_TEST);
    }
    if (stencilTestEnabled) {
      glEnable(GL_STENCIL_TEST);
    }
    // restore matrices
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

};

#endif
