/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.h
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#ifndef __TutorialApplication_h_
#define __TutorialApplication_h_

#include "BaseApplication.h"
#include "NativeRenderSystemCommands.h"

#include "../RendererPlugin.h"

#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

#include "../iphone_orientation_plugin/iphone_orientation_listener.h"

class Ogre3DRendererPlugin : public BaseApplication, public RendererPlugin
{
private:
  void setupRTT();
  Ogre::TexturePtr rtt_texture;
  Ogre::RenderTexture *renderTexture;
  Ogre::TexturePtr rtt_texture2;
  Ogre::RenderTexture *renderTexture2;

private:
  Ogre::TerrainGlobalOptions* mTerrainGlobals;
  Ogre::TerrainGroup* mTerrainGroup;
  bool mTerrainsImported;

  void defineTerrain(long x, long y);
  void initBlendMaps(Ogre::Terrain* terrain);
  void configureTerrainDefaults(Ogre::Light* light);
  bool frameRenderingQueued(const Ogre::FrameEvent& evt);

  GLuint desktopTexture;

public:
    Ogre3DRendererPlugin(Display *dpy, unsigned long screen, Window window, XVisualInfo *visualinfo, unsigned long context);
    virtual ~Ogre3DRendererPlugin();

    void init() {
      go();
//      mCamera->lookAt(-100, 0, 0);
    }
    void setDesktopTexture(GLuint desktopTexture_);

    Window getWindowID() {
      return this->windowId;
    }
    void render(double timeDiff_);
    void step(const Desktop3DLocation &loc_, double timeDiff_) {
      Ogre::FrameEvent f;
      f.timeSinceLastEvent = timeDiff_;
      f.timeSinceLastFrame = timeDiff_;
      mCameraMan->frameRenderingQueued(f);

      const double *orientation = get_orientation();
      Ogre::Matrix4 m(
          orientation[0],orientation[1],orientation[2],orientation[3],
          orientation[4],orientation[5],orientation[6],orientation[7],
          orientation[8],orientation[9],orientation[10],orientation[11],
          orientation[12],orientation[13],orientation[14],orientation[15]);
      mCamera2->setCustomViewMatrix(true, m.concatenateAffine(mCamera->getViewMatrix()));

      render(timeDiff_);
    }
    void move(int forward_, int right_, bool jump_, double relativeMouseX_, double relativeMouseY_)
    {
      OIS::Axis x;
      OIS::Axis y;
      y.rel = relativeMouseX_;
      x.rel = relativeMouseY_;
      OIS::MouseState ms;
      ms.width = width;
      ms.height = height;
      ms.X = x;
      ms.Y = y;
      OIS::MouseEvent mEvent(0, ms);
      mCameraMan->injectMouseMove(mEvent);

      if(forward_ > 0) {
        OIS::KeyEvent evt(0, OIS::KC_W, 'W');
        mCameraMan->injectKeyDown(evt);
      } else {
        OIS::KeyEvent evt(0, OIS::KC_W, 'W');
        mCameraMan->injectKeyUp(evt);
      }
      if(forward_ < 0) {
        OIS::KeyEvent evt(0, OIS::KC_S, 'S');
        mCameraMan->injectKeyDown(evt);
      } else {
        OIS::KeyEvent evt(0, OIS::KC_S, 'S');
        mCameraMan->injectKeyUp(evt);
      }

      if(right_ > 0) {
        OIS::KeyEvent evt(0, OIS::KC_D, 'D');
        mCameraMan->injectKeyDown(evt);
      } else {
        OIS::KeyEvent evt(0, OIS::KC_D, 'D');
        mCameraMan->injectKeyUp(evt);
      }
      if(right_ < 0) {
        OIS::KeyEvent evt(0, OIS::KC_A, 'A');
        mCameraMan->injectKeyDown(evt);
      } else {
        OIS::KeyEvent evt(0, OIS::KC_A, 'A');
        mCameraMan->injectKeyUp(evt);
      }
    }
    bool needsSwapBuffers() {
      return false;
    }
    void processEvents() {
      Ogre::WindowEventUtilities::messagePump();
    }

protected:
    OpenGLNativeRenderSystemCommandsRenderQueueListener * mRenderSystemCommandsRenderQueueListener;
    virtual void createScene();
    void createDesktopObject();
};

#endif // #ifndef __TutorialApplication_h_
