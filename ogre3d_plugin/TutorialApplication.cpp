/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
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
#include "TutorialApplication.h"

#include <OgreHardwarePixelBuffer.h>

#include <RenderSystems/GL/OgreGLTexture.h>
#include <RenderSystems/GL/OgreGLTextureManager.h>

using namespace Ogre;

//-------------------------------------------------------------------------------------
Ogre3DRendererPlugin::Ogre3DRendererPlugin(Display *dpy, unsigned long screen, Window window, XVisualInfo *visualinfo, unsigned long context)
: BaseApplication(dpy, screen, window, visualinfo, context)
{
}
//-------------------------------------------------------------------------------------
Ogre3DRendererPlugin::~Ogre3DRendererPlugin()
{
}

void Ogre3DRendererPlugin::setDesktopTexture(GLuint desktopTexture_) {
  this->desktopTexture = desktopTexture_;
  mRenderSystemCommandsRenderQueueListener->desktopTexture = desktopTexture_;
}
void Ogre3DRendererPlugin::render() {
//  mWindow->getViewport(0)->setClearEveryFrame(false,0);
  std::cerr << "*DRAW" << std::endl;
  bool needsInit = true;
  if(needsInit) {
      needsInit = false;
      createDesktopObject();
  }
  bool r = mRoot->renderOneFrame();
  if(!r) {
    std::cerr << "FAILED OT RENDER" << std::endl;
  }
//  material->touch();
//  material->reload();
}

//-------------------------------------------------------------------------------------
void Ogre3DRendererPlugin::createDesktopObject() {
  //GLTexturePtr t = GLTextureManager::getSingleton().createManual("DynamicTexture", // name
  //    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
  //    Ogre::TEX_TYPE_2D,      // type
  //    1280, 800,         // width & height
  //    0,                // number of mipmaps
  //    Ogre::PF_BYTE_RGBA,     // pixel format
  //    Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);//TU_DEFAULT);
    GLTexturePtr t = GLTextureManager::getSingleton().createManual("DynamicTexture", // name
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D,      // type
        1280, 800,         // width & height
        1, //depth
        MIP_DEFAULT,//0,                // number of mipmaps
        Ogre::PF_BYTE_RGBA,     // pixel format
        Ogre::TU_DEFAULT,// TU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
        0);//TU_DEFAULT);
  t->_fireLoadingComplete(true);
  std::cerr << "**** DESKTOP TEXTURE: " << this->desktopTexture << std::endl;
  desktopTexture = t->getHandle();//getGLID();//texture->getHandle();
  std::cerr << "**** DESKTOP TEXTURE: " << this->desktopTexture << std::endl;
  return;

// Create a material using the texture
 /*Ogre::MaterialPtr */material = Ogre::MaterialManager::getSingleton().create(
     "DynamicTextureMaterial", // name
     Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

 material->getTechnique(0)->getPass(0)->createTextureUnitState("DynamicTexture");
 material->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_REPLACE);//SBT_TRANSPARENT_ALPHA);
//return;
//  ogreHead->setMaterial(material);

 Ogre::ManualObject* manual = mSceneMgr->createManualObject("manual");
//  manual->begin("DynamicTextureMaterial", Ogre::RenderOperation::OT_TRIANGLE_STRIP, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
 manual->begin("BlankWhiteMaterial", Ogre::RenderOperation::OT_TRIANGLE_STRIP, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

 manual->textureCoord(0, 0);
 manual->position(-40.0, -40.0, 0.0);  // start position
 manual->textureCoord(1, 0);
 manual->position( 40.0, -40.0, 0.0);  // draw first line
 manual->textureCoord(0, 1);
 manual->position(-40.0, 40.0, 0.0);
 manual->textureCoord(1, 1);
 manual->position(40.0,  40.0, 0.0);

 manual->end();
//  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(manual);


  ManualObject *manObj; // we will use this Manual Object as a reference point for the native rendering
  manObj = mSceneMgr->createManualObject("sampleArea");

  // Attach to child of root node, better for culling (otherwise bounds are the combination of the 2)
  mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(manObj);

  String RenderSystemName = mSceneMgr->getDestinationRenderSystem()->getName();
  mRenderSystemCommandsRenderQueueListener = NULL;
  if ("OpenGL Rendering Subsystem" == RenderSystemName) {
    mRenderSystemCommandsRenderQueueListener =
        new OpenGLNativeRenderSystemCommandsRenderQueueListener(
        manObj, mCamera, mSceneMgr);

    mSceneMgr->addRenderQueueListener(mRenderSystemCommandsRenderQueueListener);
  }
  std::cerr << "***** DONE CONFIGURE and INIT SCENE" << std::endl;
  }
void Ogre3DRendererPlugin::createScene(void)
{
  // Create the texture
//  Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(
//      "DynamicTexture", // name
//      Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
//      Ogre::TEX_TYPE_2D,      // type
//      1280, 800,         // width & height
//      0,                // number of mipmaps
//      Ogre::PF_BYTE_BGRA,     // pixel format
//      Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);//TU_DEFAULT);      // usage; should be TU_DYNAMIC_WRITE_ONLY_DISCARDABLE for
//                        // textures updated very often (e.g. each frame)
//  GLTexturePtr t(texture);

    // create your scene here :)
  Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");
  Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("HeadNode");
  headNode->attachObject(ogreHead);
  Ogre::Light* light = mSceneMgr->createLight( "MainLight" );
  light->setPosition(20, 80, 50);

  return;

  createDesktopObject();
}

//   void TutorialApplication::destroyScene()
//   {
//     if (mRenderSystemCommandsRenderQueueListener)
//     {
//       mSceneMgr->removeRenderQueueListener(
//           mRenderSystemCommandsRenderQueueListener);
//
//       delete mRenderSystemCommandsRenderQueueListener;
//       mRenderSystemCommandsRenderQueueListener = NULL;
//     }
//   }

