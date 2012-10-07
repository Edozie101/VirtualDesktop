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

class Ogre3DRendererPlugin : public BaseApplication, public RendererPlugin
{
public:
//    Ogre::ResourceHandle desktopTexture;
  GLuint desktopTexture;

    Ogre3DRendererPlugin(Display *dpy, unsigned long screen, Window window, XVisualInfo *visualinfo, unsigned long context);
    virtual ~Ogre3DRendererPlugin();

    void init() {
      go();
    }
    void setDesktopTexture(GLuint desktopTexture_);

    Window getWindowID() {
      return this->windowId;
    }
    void render();
    void step(const Desktop3DLocation &loc_, double timeDiff_) {
      render();
    }
    bool needsSwapBuffers() {
      return true;
    }
    void processEvents() {
      Ogre::WindowEventUtilities::messagePump();
    }

protected:
    OpenGLNativeRenderSystemCommandsRenderQueueListener * mRenderSystemCommandsRenderQueueListener;
    virtual void createScene(void);
    void createDesktopObject();
};

#endif // #ifndef __TutorialApplication_h_
