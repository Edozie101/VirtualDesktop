/** Example 002 Quake3Map

 This Tutorial shows how to load a Quake 3 map into the engine, create a
 SceneNode for optimizing the speed of rendering, and how to create a user
 controlled camera.

 Please note that you should know the basics of the engine before starting this
 tutorial. Just take a short look at the first tutorial, if you haven't done
 this yet: http://irrlicht.sourceforge.net/tut001.html

 Lets start like the HelloWorld example: We include the irrlicht header files
 and an additional file to be able to ask the user for a driver type using the
 console.
 */
#include <irrlicht.h>
#include <iostream>

#include "../iphone_orientation_plugin/iphone_orientation_listener.h"
#include "../opengl_setup_x11.h"
#include "../ibex.h"
#include "../opengl_helpers.h"

#include "irrlicht_plugin.h"

/*
 As already written in the HelloWorld example, in the Irrlicht Engine everything
 can be found in the namespace 'irr'. To get rid of the irr:: in front of the
 name of every class, we tell the compiler that we use that namespace from now
 on, and we will not have to write that 'irr::'. There are 5 other sub
 namespaces 'core', 'scene', 'video', 'io' and 'gui'. Unlike in the HelloWorld
 example, we do not call 'using namespace' for these 5 other namespaces, because
 in this way you will see what can be found in which namespace. But if you like,
 you can also include the namespaces like in the previous example.
 */
using namespace irr;

static IrrlichtDevice *device;
static video::IVideoDriver* driver;
static scene::ISceneManager* smgr;

static GLboolean depthTestEnabled;
static GLboolean stencilTestEnabled;

static s32 newMaterialType1 = 0;
static s32 newMaterialType2 = 0;

IrrlichtRendererPlugin::IrrlichtRendererPlugin()
{
  irrlicht_plugin();
}
IrrlichtRendererPlugin::~IrrlichtRendererPlugin()
{
}

Window
IrrlichtRendererPlugin::getWindowID()
{
  return (Window) driver->getExposedVideoData().OpenGLLinux.X11Window;
}
void
IrrlichtRendererPlugin::move(int forward_, int right_, bool jump_,
    double relativeMouseX_, double relativeMouseY_)
{
  irrlicht_move(forward_, right_, jump_, relativeMouseX_, relativeMouseY_);
}
void
IrrlichtRendererPlugin::step(const Desktop3DLocation &loc, double timeDiff_)
{
  irrlicht_step(loc);
}

enum
{
  // I use this ISceneNode ID to indicate a scene node that is
  // not pickable by getSceneNodeAndCollisionPointFromRay()
  ID_IsNotPickable = 0,

  // I use this flag in ISceneNode IDs to indicate that the
  // scene node can be picked by ray selection.
  IDFlag_IsPickable = 1 << 0,

  // I use this flag in ISceneNode IDs to indicate that the
  // scene node can be highlighted.  In this example, the
  // homonids can be highlighted, but the level mesh can't.
  IDFlag_IsHighlightable = 1 << 1
};
/*
 Again, to be able to use the Irrlicht.DLL file, we need to link with the
 Irrlicht.lib. We could set this option in the project settings, but to make it
 easy, we use a pragma comment lib:
 */
#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

static void
saveState()
{
  //    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
  //======================================================================
  //rendering the raw opengl code in a custom scene node
  // save matrices
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glLoadIdentity(); //Texture addressing should start out as direct.
  depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
  glDisable(GL_DEPTH_TEST);
  stencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
  glDisable(GL_STENCIL_TEST);
  // save attribs
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  // call native rendering function
  //////////////////
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
}
static void
restoreState()
{
  ////////////////
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

class CDesktopSceneNode : public scene::ISceneNode
{

  /*
   First, we declare some member variables:
   The bounding box, 4 vertices, and the material of the tetraeder.
   */
  core::aabbox3d<f32> Box;
  video::S3DVertex Vertices[4];
  video::SMaterial Material;

  static const double yOffset = -20;

  /*
   The parameters of the constructor specify the parent of the scene node,
   a pointer to the scene manager, and an id of the scene node.
   In the constructor we call the parent class' constructor,
   set some properties of the material, and
   create the 4 vertices of the tetraeder we will draw later.
   */
public:

  CDesktopSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr,
      s32 id) :
      scene::ISceneNode(parent, mgr, id)
  {
    Material.Wireframe = false;
    Material.Lighting = false;
    Material.MaterialType = irr::video::EMT_SOLID;

    Vertices[0] = video::S3DVertex(0, 0, 10, 1, 1, 0,
        video::SColor(255, 0, 255, 255), 0, 1);
    Vertices[1] = video::S3DVertex(10, 0, -10, 1, 0, 0,
        video::SColor(255, 255, 0, 255), 1, 1);
    Vertices[2] = video::S3DVertex(0, 20, 0, 0, 1, 1,
        video::SColor(255, 255, 255, 0), 1, 0);
    Vertices[3] = video::S3DVertex(-10, 0, -10, 0, 0, 1,
        video::SColor(255, 0, 255, 0), 0, 0);

    /*
     The Irrlicht Engine needs to know the bounding box of a scene node.
     It will use it for automatic culling and other things. Hence, we
     need to create a bounding box from the 4 vertices we use.
     If you do not want the engine to use the box for automatic culling,
     and/or don't want to create the box, you could also call
     irr::scene::ISceneNode::setAutomaticCulling() with irr::scene::EAC_OFF.
     */
//                Box.reset(Vertices[0].Pos);
//                for (s32 i=1; i<4; ++i)
//                        Box.addInternalPoint(Vertices[i].Pos);
//                Box.reset(Vertices[0].Pos);
    Box.reset(core::vector3df(-80, -80 + yOffset, 150 - 0));
    Box.addInternalPoint(core::vector3df(80, 0 + yOffset, 150 - 0));
    Box.addInternalPoint(core::vector3df(-80, 80 + yOffset, 150 - 0));
    Box.addInternalPoint(core::vector3df(80, 80 + yOffset, 150 - 0));
  }

  /*
   Before it is drawn, the irr::scene::ISceneNode::OnRegisterSceneNode()
   method of every scene node in the scene is called by the scene manager.
   If the scene node wishes to draw itself, it may register itself in the
   scene manager to be drawn. This is necessary to tell the scene manager
   when it should call irr::scene::ISceneNode::render(). For
   example, normal scene nodes render their content one after another,
   while stencil buffer shadows would like to be drawn after all other
   scene nodes. And camera or light scene nodes need to be rendered before
   all other scene nodes (if at all). So here we simply register the
   scene node to render normally. If we would like to let it be rendered
   like cameras or light, we would have to call
   SceneManager->registerNodeForRendering(this, SNRT_LIGHT_AND_CAMERA);
   After this, we call the actual
   irr::scene::ISceneNode::OnRegisterSceneNode() method of the base class,
   which simply lets also all the child scene nodes of this node register
   themselves.
   */
  virtual void
  OnRegisterSceneNode()
  {
    if (IsVisible) SceneManager->registerNodeForRendering(this);

    ISceneNode::OnRegisterSceneNode();
  }

  /*
   In the render() method most of the interesting stuff happens: The
   Scene node renders itself. We override this method and draw the
   tetraeder.
   */
  virtual void
  render()
  {
    video::IVideoDriver* driver = SceneManager->getVideoDriver();

    static video::ITexture *tex = driver->getTexture(
        "./resources/humus-skybox/posy.jpg");
    Material.setTexture(0, tex);

    u16 indices[] =
      { 0, 2, 3, 2, 1, 3, 1, 0, 3, 2, 0, 1 };

    driver->setMaterial(Material);
    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

    glPushMatrix();
    glDisable(GL_CULL_FACE);
    glTranslated(0, 0, 150);
//          glRotated(-90, 0, 0, 1);
    glScaled(80, 1, 80);
    double ySize = ((double) height / (double) width) / 2.0 * 80;
    const double monitorOriginZ = -0.5;
    glBindTexture(GL_TEXTURE_2D, desktopTexture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2d(0, 0);
    glVertex3f(-0.5, -ySize + yOffset, monitorOriginZ);

    glTexCoord2d(1, 0);
    glVertex3f(0.5, -ySize + yOffset, monitorOriginZ);

    glTexCoord2d(0, 1);
    glVertex3f(-0.5, ySize + yOffset, monitorOriginZ);

    glTexCoord2d(1, 1);
    glVertex3f(0.5, ySize + yOffset, monitorOriginZ);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
  }

  /*
   And finally we create three small additional methods.
   irr::scene::ISceneNode::getBoundingBox() returns the bounding box of
   this scene node, irr::scene::ISceneNode::getMaterialCount() returns the
   amount of materials in this scene node (our tetraeder only has one
   material), and irr::scene::ISceneNode::getMaterial() returns the
   material at an index. Because we have only one material here, we can
   return the only one material, assuming that no one ever calls
   getMaterial() with an index greater than 0.
   */
  virtual const core::aabbox3d<f32>&
  getBoundingBox() const
  {
    return Box;
  }

  virtual u32
  getMaterialCount() const
  {
    return 1;
  }

  virtual video::SMaterial&
  getMaterial(u32 i)
  {
    return Material;
  }
};

static scene::ISceneNode* skyboxNode;
static scene::IMeshSceneNode* quake3LevelNode = 0;
static CDesktopSceneNode *desktopNode = 0;

static video::ITexture* rtLeft = 0;
static video::ITexture* rtRight = 0;
static scene::ICameraSceneNode* fixedCamera = 0;
static scene::ICameraSceneNode* mainCamera = 0;
/*
 Ok, lets start. Again, we use the main() method as start, not the WinMain().
 */
int
irrlicht_plugin()
{
  /*
   Like in the HelloWorld example, we create an IrrlichtDevice with
   createDevice(). The difference now is that we ask the user to select
   which video driver to use. The Software device might be
   too slow to draw a huge Quake 3 map, but just for the fun of it, we make
   this decision possible, too.
   Instead of copying this whole code into your app, you can simply include
   driverChoice.h from Irrlicht's include directory. The function
   driverChoiceConsole does exactly the same.
   */

  // ask user for driver
  video::E_DRIVER_TYPE driverType;

  printf("Please select the driver you want for this example:\n"
      " (a) OpenGL 1.5\n (b) Direct3D 9.0c\n (c) Direct3D 8.1\n"
      " (d) Burning's Software Renderer\n (e) Software Renderer\n"
      " (f) NullDevice\n (otherKey) exit\n\n");

  char i;
//        std::cin >> i;
  i = 'a';

  switch (i) {
  case 'a':
    driverType = video::EDT_OPENGL;
    break;
  case 'b':
    driverType = video::EDT_DIRECT3D9;
    break;
  case 'c':
    driverType = video::EDT_DIRECT3D8;
    break;
  case 'd':
    driverType = video::EDT_BURNINGSVIDEO;
    break;
  case 'e':
    driverType = video::EDT_SOFTWARE;
    break;
  case 'f':
    driverType = video::EDT_NULL;
    break;
  default:
    return 1;
  }

  // create device and exit if creation failed
  device = createDevice(driverType, core::dimension2d<u32>(1280,800), 32,//width, height), 32, //1440, 900), 32,
      true);

  if (device == 0) return 1; // could not create selected driver.

  /*
   Get a pointer to the video driver and the SceneManager so that
   we do not always have to call irr::IrrlichtDevice::getVideoDriver() and
   irr::IrrlichtDevice::getSceneManager().
   */
  driver = device->getVideoDriver();
  smgr = device->getSceneManager();

  context = (GLXContext) driver->getExposedVideoData().OpenGLLinux.X11Context;
  window = (Window) driver->getExposedVideoData().OpenGLLinux.X11Window;
  display = (Display*) driver->getExposedVideoData().OpenGLLinux.X11Display;

  ///////////////////////////////
  // create render target
  if (driver->queryFeature(video::EVDF_RENDER_TO_TARGET)) {
    rtLeft = driver->addRenderTargetTexture(core::dimension2d<u32>(1280,1600),//640, 800),
        "RTTLeft");
    rtRight = driver->addRenderTargetTexture(core::dimension2d<u32>(1280,1600),//640, 800),
        "RTTRight");
//          test->setMaterialTexture(0, rt); // set material of cube to render target

    // add fixed camera
    fixedCamera = smgr->addCameraSceneNode(0, core::vector3df(0, 0, -10), //10,10,-80),
    core::vector3df(0, 0, 0)); //-10,10,-100));
    core::matrix4 m;
    fixedCamera->setProjectionMatrix(
        m.buildProjectionMatrixOrthoLH(1440.0, 900.0, -1000.0, 1000.0), true);
  }
  else {
    std::cerr << "** ERROR: SYSTEM DOESN'T SUPPORT RENDER TO TEXTURE"
        << std::endl;
  }
  ///////////////////////////////
  /*
   To display the Quake 3 map, we first need to load it. Quake 3 maps
   are packed into .pk3 files which are nothing else than .zip files.
   So we add the .pk3 file to our irr::io::IFileSystem. After it was added,
   we are able to read from the files in that archive as if they are
   directly stored on the disk.
   */
  device->getFileSystem()->addZipFileArchive(
      "./resources/irrlicht-media/map-20kdm2.pk3");

  /*
   Now we can load the mesh by calling
   irr::scene::ISceneManager::getMesh(). We get a pointer returned to an
   irr::scene::IAnimatedMesh. As you might know, Quake 3 maps are not
   really animated, they are only a huge chunk of static geometry with
   some materials attached. Hence the IAnimatedMesh consists of only one
   frame, so we get the "first frame" of the "animation", which is our
   quake level and create an Octree scene node with it, using
   irr::scene::ISceneManager::addOctreeSceneNode().
   The Octree optimizes the scene a little bit, trying to draw only geometry
   which is currently visible. An alternative to the Octree would be a
   irr::scene::IMeshSceneNode, which would always draw the complete
   geometry of the mesh, without optimization. Try it: Use
   irr::scene::ISceneManager::addMeshSceneNode() instead of
   addOctreeSceneNode() and compare the primitives drawn by the video
   driver. (There is a irr::video::IVideoDriver::getPrimitiveCountDrawn()
   method in the irr::video::IVideoDriver class). Note that this
   optimization with the Octree is only useful when drawing huge meshes
   consisting of lots of geometry.
   */
  scene::IAnimatedMesh* mesh = smgr->getMesh("20kdm2.bsp");
//  scene::ISceneNode* node = 0;

  if (mesh)
    quake3LevelNode = smgr->addOctreeSceneNode(mesh->getMesh(0), 0,
        IDFlag_IsPickable, 1024); //-1, 1024);

//              node = smgr->addMeshSceneNode(mesh->getMesh(0));

  /*
   Because the level was not modelled around the origin (0,0,0), we
   translate the whole level a little bit. This is done on
   irr::scene::ISceneNode level using the methods
   irr::scene::ISceneNode::setPosition() (in this case),
   irr::scene::ISceneNode::setRotation(), and
   irr::scene::ISceneNode::setScale().
   */
  if (quake3LevelNode)
    quake3LevelNode->setPosition(core::vector3df(-1300 + 50, -144 - 20, -1249));

  scene::ITriangleSelector* selector = 0;
  if (quake3LevelNode) {
    selector = smgr->createOctreeTriangleSelector(quake3LevelNode->getMesh(),
        quake3LevelNode, 128);
    quake3LevelNode->setTriangleSelector(selector);
  }

  /*
   Now we only need a camera to look at the Quake 3 map.
   We want to create a user controlled camera. There are some
   cameras available in the Irrlicht engine. For example the
   MayaCamera which can be controlled like the camera in Maya:
   Rotate with left mouse button pressed, Zoom with both buttons pressed,
   translate with right mouse button pressed. This could be created with
   irr::scene::ISceneManager::addCameraSceneNodeMaya(). But for this
   example, we want to create a camera which behaves like the ones in
   first person shooter games (FPS) and hence use
   irr::scene::ISceneManager::addCameraSceneNodeFPS().
   */
  mainCamera = smgr->addCameraSceneNodeFPS(0, 100.0f, .3f, ID_IsNotPickable, 0,
      0, true, 3.f); //,false, true);
// mainCamera =smgr->addCameraSceneNode(0, core::vector3df(0, 0, 0), core::vector3df(0, 0, -10));

  /*
   The mouse cursor needs not be visible, so we hide it via the
   irr::IrrlichtDevice::ICursorControl.
   */
  device->getCursorControl()->setVisible(false);

  ///////////////////////////////////////////////

  mainCamera->setFOV(110.0 / 180.0 * M_PI);
  mainCamera->setAspectRatio(110. / 90.);

//  smgr->getActiveCamera()->setID(ID_IsNotPickable);

//  smgr->addCameraSceneNode(0, core::vector3df(0,-40,0), core::vector3df(0,0,0));

  // create sky box
  driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
  skyboxNode = smgr->addSkyBoxSceneNode(
      driver->getTexture("./resources/irrlicht-media/irrlicht2_up.jpg"),
      driver->getTexture("./resources/irrlicht-media/irrlicht2_dn.jpg"),
      driver->getTexture("./resources/irrlicht-media/irrlicht2_lf.jpg"),
      driver->getTexture("./resources/irrlicht-media/irrlicht2_rt.jpg"),
      driver->getTexture("./resources/irrlicht-media/irrlicht2_ft.jpg"),
      driver->getTexture("./resources/irrlicht-media/irrlicht2_bk.jpg"));
  driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

  /*
   Create our scene node. I don't check the result of calling new, as it
   should throw an exception rather than returning 0 on failure. Because
   the new node will create itself with a reference count of 1, and then
   will have another reference added by its parent scene node when it is
   added to the scene, I need to drop my reference to it. Best practice is
   to drop it only *after* I have finished using it, regardless of what
   the reference count of the object is after creation.
   */
  desktopNode = new CDesktopSceneNode(smgr->getRootSceneNode(), smgr, 666);

  /*
   To animate something in this boring scene consisting only of one
   tetraeder, and to show that you now can use your scene node like any
   other scene node in the engine, we add an animator to the scene node,
   which rotates the node a little bit.
   irr::scene::ISceneManager::createRotationAnimator() could return 0, so
   should be checked.
   */
  scene::ISceneNodeAnimator* anim = smgr->createRotationAnimator(
      core::vector3df(0.8f, 0, 0.8f));

  if (anim) {
//            myNode->addAnimator(anim);

    /*
     I'm done referring to anim, so must
     irr::IReferenceCounted::drop() this reference now because it
     was produced by a createFoo() function. As I shouldn't refer to
     it again, ensure that I can't by setting to 0.
     */
    anim->drop();
    anim = 0;
  }

//  /*
//   I'm done with my CSampleSceneNode object, and so must drop my reference.
//   This won't delete the object, yet, because it is still attached to the
//   scene graph, which prevents the deletion until the graph is deleted or the
//   custom scene node is removed from it.
//   */
//  desktopNode->drop();
//  desktopNode = 0; // As I shouldn't refer to it again, ensure that I can't

//  scene::ISceneNodeAnimator* anim2 = smgr->createCollisionResponseAnimator(
//          selector,
//          camera);
//    camera->addAnimator(anim2);
//
  if (selector) {
    scene::ISceneNodeAnimator* anim2 = smgr->createCollisionResponseAnimator(
        selector, mainCamera, core::vector3df(30, 50, 30),
        core::vector3df(0, -10, 0), core::vector3df(0, 30, 0));
    selector->drop(); // As soon as we're done with the selector, drop it.
    mainCamera->addAnimator(anim2);
    anim2->drop(); // And likewise, drop the animator when we're done referring to it.
  }

  return 0;
}

void
checkCollisions(Desktop3DLocation& loc)
{
  // Remember which scene node is highlighted
  static scene::ISceneNode* highlightedSceneNode = 0;
  static scene::ISceneCollisionManager* collMan =
      smgr->getSceneCollisionManager();

  // Unlight any currently highlighted scene node
  if (highlightedSceneNode) {
    highlightedSceneNode->setMaterialFlag(video::EMF_LIGHTING, true);
    highlightedSceneNode = 0;
  }

  // All intersections in this example are done with a ray cast out from the camera to
  // a distance of 1000.  You can easily modify this to check (e.g.) a bullet
  // trajectory or a sword's position, or create a ray from a mouse click position using
  // ISceneCollisionManager::getRayFromScreenCoordinates()
  core::line3d<f32> ray;
  ray.start = mainCamera->getPosition();
  ray.end = ray.start
      + (mainCamera->getTarget() - ray.start).normalize() * 1000.0f;

  // Tracks the current intersection point with the level or a mesh
  core::vector3df intersection;
  // Used to show with triangle has been hit
  core::triangle3df hitTriangle;

  // This call is all you need to perform ray/triangle collision on every scene node
  // that has a triangle selector, including the Quake level mesh.  It finds the nearest
  // collision point/triangle, and returns the scene node containing that point.
  // Irrlicht provides other types of selection, including ray/triangle selector,
  // ray/box and ellipse/triangle selector, plus associated helpers.
  // See the methods of ISceneCollisionManager
  scene::ISceneNode * selectedSceneNode =
      collMan->getSceneNodeAndCollisionPointFromRay(ray, intersection, // This will be the position of the collision
          hitTriangle, // This will be the triangle hit in the collision
          IDFlag_IsPickable, // This ensures that only nodes that we have
          // set up to be pickable are considered
          0);// Check the entire scene (this is actually the implicit default)

  // If the ray hit anything, move the billboard to the collision position
  // and draw the triangle that was hit.
  if (selectedSceneNode) {
    //bill->setPosition(intersection);

    // We need to reset the transform before doing our own rendering.
    driver->setTransform(video::ETS_WORLD, core::matrix4());
    //driver->setMaterial(material);
    driver->draw3DTriangle(hitTriangle, video::SColor(0, 255, 0, 0));

    // We can check the flags for the scene node that was hit to see if it should be
    // highlighted. The animated nodes can be highlighted, but not the Quake level mesh
    if ((selectedSceneNode->getID() & IDFlag_IsHighlightable)
        == IDFlag_IsHighlightable) {
      highlightedSceneNode = selectedSceneNode;

      // Highlighting in this case means turning lighting OFF for this node,
      // which means that it will be drawn with full brightness.
      highlightedSceneNode->setMaterialFlag(video::EMF_LIGHTING, false);
    }
  }
}
void
irrlicht_step(const Desktop3DLocation& loc)
{
  // create test cube
  static bool init = true;
  static scene::ISceneNode* leftEye = smgr->addBillboardSceneNode(
  smgr->getRootSceneNode(), core::dimension2df(720,900));
//  smgr->getRootSceneNode(), core::dimension2df(1,1));//720, 900));
  static scene::ISceneNode* rightEye = smgr->addBillboardSceneNode(
      smgr->getRootSceneNode(), core::dimension2df(720,900));//1,1));//720, 900));
  if (init) {
    init = false;

    loadShaders();

    leftEye->setPosition(core::vector3df(-1440.0 / 4.0, 0, 0)); //-1440.0/2.0,0,-1440.0/2.0));///2.0));
    leftEye->setMaterialFlag(video::EMF_LIGHTING, false); // disable dynamic lighting
    leftEye->setMaterialTexture(0, rtLeft); // set material of cube to render target
//    leftEye->setMaterialType((video::E_MATERIAL_TYPE)newMaterialType1);
//
//    leftEye->setMaterialFlag( irr::video::EMF_BILINEAR_FILTER, false );
//    leftEye->setMaterialFlag( irr::video::EMF_TRILINEAR_FILTER, true );
//    leftEye->setMaterialFlag( irr::video::EMF_ANISOTROPIC_FILTER, true );

    rightEye->setPosition(core::vector3df(1440.0 / 4.0, 0, 0)); //1440.0/2.0));
    rightEye->setMaterialFlag(video::EMF_LIGHTING, false); // disable dynamic lighting
    rightEye->setMaterialTexture(0, rtRight); // set material of cube to render target
//    rightEye->setMaterialType((video::E_MATERIAL_TYPE)newMaterialType2);

  }

  device->run();

  static u32 then = device->getTimer()->getTime();
  // This is the movemen speed in units per second.
  static const f32 MOVEMENT_SPEED = 5.f;

  // Work out a frame delta time.
  const u32 now = device->getTimer()->getTime();
  const f32 frameDeltaTime = (f32) (now - then) / 1000.f; // Time in seconds
  then = now;

//  loc.setWalkSpeed(100);
  const core::vector3df position(-loc.getXPosition(), loc.getYPosition(),
      loc.getZPosition());

  // Direction : Spherical coordinates to Cartesian coordinates conversion
  const double verticalAngle = loc.getXRotation() / 90.0 * M_PI_2;
  const double horizontalAngle = loc.getYRotation() / 90.0 * M_PI_2;
  const core::vector3df direction(
  /*cos(verticalAngle) * */sin(horizontalAngle) * 100,
      -sin(verticalAngle) * 100,
      /*cos(verticalAngle) * */cos(horizontalAngle) * 100);

//  smgr->getActiveCamera()->setPosition(position);
//  smgr->getActiveCamera()->setTarget(position + direction);

  core::vector3df p = smgr->getActiveCamera()->getPosition();
//  p.X += MOVEMENT_SPEED*frameDeltaTime*direction.X;
////  p.Y += MOVEMENT_SPEED*frameDeltaTime*direction.Y;
//  p.Z += MOVEMENT_SPEED*frameDeltaTime*direction.Z;
//  smgr->getActiveCamera()->setPosition(p);
  smgr->getActiveCamera()->setTarget(p + direction);

  /*
   We have done everything, so lets draw it. We also write the current
   frames per second and the primitives drawn into the caption of the
   window. The test for irr::IrrlichtDevice::isWindowActive() is optional,
   but prevents the engine to grab the mouse cursor after task switching
   when other programs are active. The call to
   irr::IrrlichtDevice::yield() will avoid the busy loop to eat up all CPU
   cycles when the window is not active.
   */
  int lastFPS = -1;

  if (didInitOpenGL()) {
    glxYInverted = true;
    if (glxYInverted) {
      top = 0.0f;
      bottom = 1.0f;
    }
    else {
      top = 1.0f;
      bottom = 0.0f;
    }
    top = -top;
    bottom = -bottom;

    saveState();
    renderDesktopToTexture();
    restoreState();
  }

  driver->beginScene(true, true, video::SColor(255, 255, 255, 255));
//  checkCollisions(loc);
  smgr->setActiveCamera(mainCamera);
  core::matrix4 m;
  m.setM(get_orientation_f());
  mainCamera->setViewMatrixAffector(m);
  skyboxNode->setVisible(true);
  quake3LevelNode->setVisible(true);
  desktopNode->setVisible(true);
  leftEye->setVisible(false);
  rightEye->setVisible(false);
  if(SBS) {
    driver->setRenderTarget(rtLeft, true, true, video::SColor(0, 0, 0, 255));
    smgr->drawAll();
    driver->setRenderTarget(rtRight, true, true, video::SColor(0, 0, 0, 255));
    smgr->drawAll();

    driver->setRenderTarget(0, true, true, video::SColor(0, 0, 0, 255));

    smgr->setActiveCamera(fixedCamera);
    skyboxNode->setVisible(false);
    quake3LevelNode->setVisible(false);
    desktopNode->setVisible(false);
    leftEye->setVisible(true);  //  leftEye->setPosition(core::vector3df(0,0,0));
    rightEye->setVisible(true); //  rightEye->setPosition(core::vector3df(0,0,0));

    smgr->drawAll();
  } else {
    smgr->drawAll();
  }
  driver->endScene();

  smgr->setActiveCamera(mainCamera);

  int fps = driver->getFPS();

  if (lastFPS != fps) {
    lastFPS = fps;
  }

  return;
}

void
irrlicht_run_loop()
{
  /*
   We have done everything, so lets draw it. We also write the current
   frames per second and the primitives drawn into the caption of the
   window. The test for irr::IrrlichtDevice::isWindowActive() is optional,
   but prevents the engine to grab the mouse cursor after task switching
   when other programs are active. The call to
   irr::IrrlichtDevice::yield() will avoid the busy loop to eat up all CPU
   cycles when the window is not active.
   */
  int lastFPS = -1;

  while (device->run()) {
    if (device->isWindowActive()) {
      driver->beginScene(true, true, video::SColor(255, 200, 200, 200));
      smgr->drawAll();
      driver->endScene();

      int fps = driver->getFPS();

      if (lastFPS != fps) {
        core::stringw str = L"Irrlicht Engine - Quake 3 Map example [";
        str += driver->getName();
        str += "] FPS:";
        str += fps;

        device->setWindowCaption(str.c_str());
        lastFPS = fps;
      }
    }
    else
      device->yield();
  }

  /*
   In the end, delete the Irrlicht device.
   */
  device->drop();
  return;
}

void
irrlicht_move(int forward, int right, bool jump, double relativeMouseX,
    double relativeMouseY)
{
  if (forward == 1) {
    SEvent e;
    e.EventType = EET_KEY_INPUT_EVENT;
    e.KeyInput.Char = 0;
    e.KeyInput.PressedDown = true;
    e.KeyInput.Control = false;
    e.KeyInput.Shift = false;
    e.KeyInput.Key = KEY_UP;
    smgr->getActiveCamera()->OnEvent(e);
  }
  else {
    SEvent e;
    e.EventType = EET_KEY_INPUT_EVENT;
    e.KeyInput.Char = 0;
    e.KeyInput.PressedDown = false;
    e.KeyInput.Control = false;
    e.KeyInput.Shift = false;
    e.KeyInput.Key = KEY_UP;
    smgr->getActiveCamera()->OnEvent(e);
  }

  if (forward == -1) {
    SEvent e;
    e.EventType = EET_KEY_INPUT_EVENT;
    e.KeyInput.Char = 0;
    e.KeyInput.PressedDown = true;
    e.KeyInput.Control = false;
    e.KeyInput.Shift = false;
    e.KeyInput.Key = KEY_DOWN;
    smgr->getActiveCamera()->OnEvent(e);
  }
  else {
    SEvent e;
    e.EventType = EET_KEY_INPUT_EVENT;
    e.KeyInput.Char = 0;
    e.KeyInput.PressedDown = false;
    e.KeyInput.Control = false;
    e.KeyInput.Shift = false;
    e.KeyInput.Key = KEY_DOWN;
    smgr->getActiveCamera()->OnEvent(e);
  }

  if (right == 1) {
    SEvent e;
    e.EventType = EET_KEY_INPUT_EVENT;
    e.KeyInput.Char = 0;
    e.KeyInput.PressedDown = true;
    e.KeyInput.Control = false;
    e.KeyInput.Shift = false;
    e.KeyInput.Key = KEY_RIGHT;
    smgr->getActiveCamera()->OnEvent(e);
  }
  else {
    SEvent e;
    e.EventType = EET_KEY_INPUT_EVENT;
    e.KeyInput.Char = 0;
    e.KeyInput.PressedDown = false;
    e.KeyInput.Control = false;
    e.KeyInput.Shift = false;
    e.KeyInput.Key = KEY_RIGHT;
    smgr->getActiveCamera()->OnEvent(e);
  }

  if (right == -1) {
    SEvent e;
    e.EventType = EET_KEY_INPUT_EVENT;
    e.KeyInput.Char = 0;
    e.KeyInput.PressedDown = true;
    e.KeyInput.Control = false;
    e.KeyInput.Shift = false;
    e.KeyInput.Key = KEY_LEFT;
    smgr->getActiveCamera()->OnEvent(e);
  }
  else {
    SEvent e;
    e.EventType = EET_KEY_INPUT_EVENT;
    e.KeyInput.Char = 0;
    e.KeyInput.PressedDown = false;
    e.KeyInput.Control = false;
    e.KeyInput.Shift = false;
    e.KeyInput.Key = KEY_LEFT;
    smgr->getActiveCamera()->OnEvent(e);
  }

  if (jump) {
    SEvent e;
    e.EventType = EET_KEY_INPUT_EVENT;
    e.KeyInput.Char = 'j';
    e.KeyInput.PressedDown = true;
    e.KeyInput.Control = false;
    e.KeyInput.Shift = false;
    e.KeyInput.Key = KEY_KEY_J;
    smgr->getActiveCamera()->OnEvent(e);
  }
  else {
    SEvent e;
    e.EventType = EET_KEY_INPUT_EVENT;
    e.KeyInput.Char = 'j';
    e.KeyInput.PressedDown = false;
    e.KeyInput.Control = false;
    e.KeyInput.Shift = false;
    e.KeyInput.Key = KEY_KEY_J;
    smgr->getActiveCamera()->OnEvent(e);
  }
//
//  SEvent e;
//  e.EventType = EET_MOUSE_INPUT_EVENT;
//  e.MouseInput.X = relativeMouseX;
//  e.MouseInput.Y = relativeMouseY;
//  smgr->getActiveCamera()->OnEvent(e);

}

class MyShaderCallBack : public video::IShaderConstantSetCallBack
{
public:

  virtual void
  OnSetConstants(video::IMaterialRendererServices* services, s32 userData)
  {
//    video::IVideoDriver* driver = services->getVideoDriver();

    // set inverted world matrix
    // if we are using highlevel shaders (the user can select this when
    // starting the program), we must set the constants by name.

//    core::matrix4 invWorld = driver->getTransform(video::ETS_WORLD);
//    invWorld.makeInverse();
//
//    services->setVertexShaderConstant("mInvWorld", invWorld.pointer(), 16);

    // set clip matrix

//    core::matrix4 worldViewProj;
//    worldViewProj = driver->getTransform(video::ETS_PROJECTION);
//    worldViewProj *= driver->getTransform(video::ETS_VIEW);
//    worldViewProj *= driver->getTransform(video::ETS_WORLD);

//    services->setVertexShaderConstant("mWorldViewProj", worldViewProj.pointer(), 16);
//    // set camera position
//
//    core::vector3df pos = device->getSceneManager()->getActiveCamera()->getAbsolutePosition();
//
//    services->setVertexShaderConstant("mLightPos", reinterpret_cast<f32*>(&pos), 3);
//
//    // set light color
//
//    video::SColorf col(0.0f, 1.0f, 1.0f, 0.0f);
//
//    services->setVertexShaderConstant("mLightColor", reinterpret_cast<f32*>(&col), 4);
//
//    // set transposed world matrix
//
//    core::matrix4 world = driver->getTransform(video::ETS_WORLD);
//    world = world.getTransposed();
//
//    services->setVertexShaderConstant("mTransWorld", world.pointer(), 16);

    f32 offset = 0;
    bool result = services->setVertexShaderConstant("offsetUniform", reinterpret_cast<f32*>(&offset), 1);
    if(!result) {
      std::cerr << "FAILED TO LOAD" << std::endl;
      exit(1);
    } else {
//      std::cerr << "LOADED " << std::endl;
    }

    //using this code inside the shader callback
    int tex_1 = 0; //the index you previously set up the texture
    result = services->setPixelShaderConstant("texture",(float*)(&tex_1),1);
    if(!result) {
      std::cerr << "FAILED TO LOAD" << std::endl;
      exit(1);
    } else {
//      std::cerr << "LOADED " << std::endl;
    }

  }
};

void
loadShaders()
{
  // create materials
  io::path vsFileName = "./resources/shaders/distortions.v.glsl"; // filename for the vertex shader
  io::path psFileName = "./resources/shaders/distortions.f.glsl"; // filename for the pixel shader
  if (!driver->queryFeature(video::EVDF_PIXEL_SHADER_1_2)
      && !driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1 )) {
    device->getLogger()->log(
        "WARNING: Pixel shaders disabled because of missing driver/hardware support.");
    psFileName = "";
  }
  if (!driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1)
      && !driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1)) {
    device->getLogger()->log(
        "WARNING: Vertex shaders disabled because of missing driver/hardware support.");
    vsFileName = "";
  }

  video::IGPUProgrammingServices* gpu = driver->getGPUProgrammingServices();
  if (gpu) {
    MyShaderCallBack* mc = new MyShaderCallBack();

    newMaterialType1 = gpu->addHighLevelShaderMaterialFromFiles(vsFileName,
        "main", video::EVST_VS_1_1,
        psFileName, "main",
        video::EPST_PS_1_2, mc, video::EMT_TRANSPARENT_ALPHA_CHANNEL);//EMT_SOLID);

    newMaterialType2 = gpu->addHighLevelShaderMaterialFromFiles(vsFileName,
        "main", video::EVST_VS_1_1, psFileName, "main",
        video::EPST_PS_1_2, mc, video::EMT_TRANSPARENT_ADD_COLOR);

    mc->drop();

    std::cerr << "********************** " << std::endl;
  }
}
