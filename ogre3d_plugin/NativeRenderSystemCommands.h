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

#ifndef __NativeRenderSystemCommands_h_
#define __NativeRenderSystemCommands_h_

// --- OpenGL ----------------------------------------------------------------
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>
#include <GL/glu.h>

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

  void
  NativeRender()

  {
//    glDisable (GL_TEXTURE_2D);

    glDisable(GL_LIGHTING);

    glDisable(GL_BLEND);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, this->desktopTexture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTranslated(-10, 0, 0);
    glScalef(10.0, 10.0, 10.0);

    static int i = 0;
    ++i;
    glRotated(i, 1.0, 0.2, 0.5);

    // I took the following sample code from here: http://www.oreillynet.com/network/2000/06/23/magazine/cube.c
    // OK, let's start drawing our planer quads.

    glBegin(GL_QUADS);
    // Bottom Face.  Red, 75% opaque, magnified texture

    glNormal3f(0.0f, -1.0f, 0.0f); // Needed for lighting

    glColor4f(0.9, 0.2, 0.2, .75); // Basic polygon color

    glTexCoord2f(0.f, 0.f);
    glVertex3f(-1.0f, -1.0f, -1.0f);

    glTexCoord2f(1.f, 0.f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    glTexCoord2f(1.f, 1.f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    glTexCoord2f(0.f, 1.f);
    glVertex3f(-1.0f, -1.0f, 1.0f);

    // Top face; offset.  White, 50% opaque.

    glNormal3f(0.0f, 1.0f, 0.0f);
    glColor4f(0.5, 0.5, 0.5, .5);

    glTexCoord2f(0.005f, 1.995f);
    glVertex3f(-1.0f, 1.3f, -1.0f);

    glTexCoord2f(0.005f, 0.005f);
    glVertex3f(-1.0f, 1.3f, 1.0f);

    glTexCoord2f(1.995f, 0.005f);
    glVertex3f(1.0f, 1.3f, 1.0f);

    glTexCoord2f(1.995f, 1.995f);
    glVertex3f(1.0f, 1.3f, -1.0f);

    // Far face.  Green, 50% opaque, non-uniform texture cooridinates.

    glNormal3f(0.0f, 0.0f, -1.0f);
    glColor4f(0.2, 0.9, 0.2, .5);

    glTexCoord2f(0.995f, 0.005f);
    glVertex3f(-1.0f, -1.0f, -1.3f);

    glTexCoord2f(2.995f, 2.995f);
    glVertex3f(-1.0f, 1.0f, -1.3f);

    glTexCoord2f(0.005f, 0.995f);
    glVertex3f(1.0f, 1.0f, -1.3f);

    glTexCoord2f(0.005f, 0.005f);
    glVertex3f(1.0f, -1.0f, -1.3f);

    // Right face.  Blue; 25% opaque

    glNormal3f(1.0f, 0.0f, 0.0f);
    glColor4f(0.2, 0.2, 0.9, .25);

    glTexCoord2f(0.995f, 0.005f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    glTexCoord2f(0.995f, 0.995f);
    glVertex3f(1.0f, 1.0f, -1.0f);

    glTexCoord2f(0.005f, 0.995f);
    glVertex3f(1.0f, 1.0f, 1.0f);

    glTexCoord2f(0.005f, 0.005f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    // Front face; offset.  Multi-colored, 50% opaque.

    glNormal3f(0.0f, 0.0f, 1.0f);

    glColor4f(0.9f, 0.2f, 0.2f, 0.5f);

    glTexCoord2f(0.f, 0.f);
    glVertex3f(-1.0f, -1.0f, 1.3f);

    glColor4f(0.2f, 0.9f, 0.2f, 0.5f);

    glTexCoord2f(1.f, 0.f);
    glVertex3f(1.0f, -1.0f, 1.3f);

    glColor4f(0.2f, 0.2f, 0.9f, 0.5f);

    glTexCoord2f(1.f, 1.f);
    glVertex3f(1.0f, 1.0f, 1.3f);

    glColor4f(0.1f, 0.1f, 0.1f, 0.5f);

    glTexCoord2f(0.f, 1.f);
    glVertex3f(-1.0f, 1.0f, 1.3f);

    // Left Face; offset.  Yellow, varying levels of opaque.

    glNormal3f(-1.0f, 0.0f, 0.0f);

    glColor4f(0.9, 0.9, 0.2, 0.0);

    glTexCoord2f(0.005f, 0.005f);
    glVertex3f(-1.3f, -1.0f, -1.0f);

    glColor4f(0.9, 0.9, 0.2, 0.66);

    glTexCoord2f(0.995f, 0.005f);
    glVertex3f(-1.3f, -1.0f, 1.0f);

    glColor4f(0.9, 0.9, 0.2, 1.0);

    glTexCoord2f(0.995f, 0.995f);
    glVertex3f(-1.3f, 1.0f, 1.0f);

    glColor4f(0.9, 0.9, 0.2, 0.33);

    glTexCoord2f(0.005f, 0.995f);
    glVertex3f(-1.3f, 1.0f, -1.0f);

    // All polygons have been drawn.

    glEnd();

  }

public:
  GLuint desktopTexture;
  OpenGLNativeRenderSystemCommandsRenderQueueListener(MovableObject* object,
      const Camera* camera, SceneManager* sceneMgr) :

      mObject(object),

      mCamera(camera),

      mSceneMgr(sceneMgr)
  {
  }

  virtual void
  renderQueueStarted(uint8 queueGroupId, const String& invocation,

  bool& skipThisInvocation)
  {
  }

  void
  blah()
  {
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
            "BaseWhite");
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
    glEnable(GL_TEXTURE_2D);

    if (didInitOpenGL())
      {
        renderDesktopToTexture();
      }

    //////////////////
    // restore original state

    glPopAttrib();
    if (depthTestEnabled)
      {
        glEnable(GL_DEPTH_TEST);
      }
    if (stencilTestEnabled)
      {
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
  virtual void
  renderQueueEnded(uint8 queueGroupId, const String& invocation,

  bool& repeatThisInvocation)

  {
    // Set wanted render queue here - make sure there are - make sure that something is on
    // this queue - else you will never pass this if.
    if (queueGroupId != RENDER_QUEUE_MAIN)
      return;

    blah();

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
            "BaseWhite");

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

    if (depthTestEnabled)

      {

        glEnable(GL_DEPTH_TEST);

      }

    if (stencilTestEnabled)

      {

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
//
//class NativeRenderSystemCommandsApplication : public SkyDomeApplication
//
//{
//
//public:
//
//  NativeRenderSystemCommandsApplication()
//
//  {
//
//  }
//
//protected:
//
//  RenderQueueListener * mRenderSystemCommandsRenderQueueListener;
//
//  // Just override the mandatory create scene method
//
//  void
//  createScene(void)
//
//  {
//
//    SkyDomeApplication::createScene();
//
//    ManualObject *manObj; // we will use this Manual Object as a reference point for the native rendering
//
//    manObj = mSceneMgr->createManualObject("sampleArea");
//
//    // Attach to child of root node, better for culling (otherwise bounds are the combination of the 2)
//
//    mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(manObj);
//
//    String RenderSystemName =
//        mSceneMgr->getDestinationRenderSystem()->getName();
//
//    mRenderSystemCommandsRenderQueueListener = NULL;
//
//    if ("OpenGL Rendering Subsystem" == RenderSystemName)
//
//    {
//
//      mRenderSystemCommandsRenderQueueListener =
//          new OpenGLNativeRenderSystemCommandsRenderQueueListener(
//
//          manObj, mCamera, mSceneMgr);
//
//      mSceneMgr->addRenderQueueListener(
//          mRenderSystemCommandsRenderQueueListener);
//
//    }
//
//  }
//
//  void
//  destroyScene()
//
//  {
//
//    if (mRenderSystemCommandsRenderQueueListener)
//
//    {
//
//      mSceneMgr->removeRenderQueueListener(
//          mRenderSystemCommandsRenderQueueListener);
//
//      delete mRenderSystemCommandsRenderQueueListener;
//
//      mRenderSystemCommandsRenderQueueListener = NULL;
//
//    }
//
//    SkyDomeApplication::destroyScene();
//
//  }
//
//};
#endif
