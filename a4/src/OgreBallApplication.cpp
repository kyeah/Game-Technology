/*
  -----------------------------------------------------------------------------
  Filename:    OgreBallApplication.cpp
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
#include "OgreBallApplication.h"

#include "ProceduralStableHeaders.h"
#include "Procedural.h"
#include "ProceduralUtils.h"

using namespace Ogre;
using namespace std;
using namespace sh;

OgreBallApplication::OgreBallApplication(void)
{
  mPhysics = new Physics(btVector3(0, -490, 0));
  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
}

//-------------------------------------------------------------------------------------
OgreBallApplication::~OgreBallApplication(void)
{
}

//-------------------------------------------------------------------------------------
void OgreBallApplication::createScene(void)
{
  mSceneMgr->setAmbientLight(Ogre::ColourValue(.5f,.5f,.5f));
  new OgreBall(mSceneMgr, "ball", "ball", "penguin.mesh", 0, mPhysics, btVector3(0, 500, 0));

  Ogre::Light* light = mSceneMgr->createLight("MainLight");
  light->setPosition(20.0f, 80.0f, 50.0f);

  levelLoader = new LevelLoader(mSceneMgr, mPhysics);
  levelLoader->loadResources("media/OgreBall/scripts");
  levelLoader->loadLevel("baseLevel");

  // GIANT COPY PASTA TEST BELOW

  // -- Ground plane
  Procedural::PlaneGenerator().setNumSegX(20).setNumSegY(20).setSizeX(150).setSizeY(150).setUTile(5.0).setVTile(5.0).realizeMesh("planeMesh");
  //  putMesh2("planeMesh");
  // -- Road
  // The path of the road, generated from a simple spline
  Procedural::Path p = Procedural::CatmullRomSpline3().setNumSeg(8).addPoint(0,0,0).addPoint(0,0,10).addPoint(10,0,10).addPoint(20,0,0).close().realizePath().scale(2);
  // The shape that will be extruded along the path
  Procedural::Shape s = Procedural::Shape().addPoint(-1.2f,.2f).addPoint(-1.f,.2f).addPoint(-.9f,.1f).addPoint(.9f,.1f).addPoint(1.f,.2f).addPoint(1.2f,.2f).scale(2).setOutSide(Procedural::SIDE_LEFT);
  // This is an example use of a shape texture track, 
  // which specifies how texture coordinates should be mapped relative to the shape points
  Procedural::Track textureTrack = Procedural::Track(Procedural::Track::AM_POINT).addKeyFrame(0,0).addKeyFrame(2,.2f).addKeyFrame(3,.8f).addKeyFrame(5,1);
  // The extruder actually creates the road mesh from all parameters
  Ogre::MeshPtr meshptr = Procedural::Extruder().setExtrusionPath(&p).setShapeToExtrude(&s).setShapeTextureTrack(&textureTrack).setUTile(20.f).realizeMesh("extrudedMesh");
  //  putMesh3("extrudedMesh");
  // -- Pillar
  // The path of the pillar, just a straight line
  Procedural::Path pillarBodyPath = Procedural::LinePath().betweenPoints(Vector3(0,0,0), Vector3(0,5,0)).realizePath();
  // We're doing something custom for the shape to extrude
  Procedural::Shape pillarBodyShape;
  const int pillarSegs=64;
  for (int i=0;i<pillarSegs;i++)                      
    pillarBodyShape.addPoint(.5*(1-.15*abs(sin(i/(float)pillarSegs*8.))) * 
                             Vector2(cos(i/(float)pillarSegs*6.28), sin(i/(float)pillarSegs*6.28)));
  pillarBodyShape.close();
  // We're also setting up a scale track, as traditionnal pillars are not perfectly straight
  Procedural::Track pillarTrack = Procedural::CatmullRomSpline2().addPoint(0,1).addPoint(0.5f,.95f).addPoint(1,.8f).realizeShape().convertToTrack(Procedural::Track::AM_RELATIVE_LINEIC);
  // Creation of the pillar body
  Procedural::TriangleBuffer pillarTB;
  Procedural::Extruder().setExtrusionPath(&pillarBodyPath).setShapeToExtrude(&pillarBodyShape).setScaleTrack(&pillarTrack).setCapped(false).setPosition(0,1,0).addToTriangleBuffer(pillarTB);
  // Creation of the top and the bottom of the pillar
  Procedural::Shape s3 = Procedural::RoundedCornerSpline2().addPoint(-1,-.25f).addPoint(-1,.25f).addPoint(1,.25f).addPoint(1,-.25f).close().realizeShape().setOutSide(Procedural::SIDE_LEFT);
  Procedural::Path p3;
  for (int i=0;i<32;i++)
    {
      Ogre::Radian r = (Ogre::Radian) (Ogre::Math::HALF_PI-(float)i/32.*Ogre::Math::TWO_PI);
      p3.addPoint(0,-.5+.5*i/32.*Math::Sin(r),-1+.5*i/32.*Math::Cos(r));
    }
  p3.addPoint(0,0,-1).addPoint(0,0,1);
  for (int i=0;i<32;i++)
    {
      Ogre::Radian r = (Ogre::Radian) (Ogre::Math::HALF_PI-(float)i/32.*Ogre::Math::TWO_PI);
      p3.addPoint(0,-.5+.5*(1-i/32.)*Math::Sin(r),1+.5*(1-i/32.)*Math::Cos(r));
    }
  Procedural::Extruder().setExtrusionPath(&p3).setShapeToExtrude(&s3).setPosition(0,6.,0).addToTriangleBuffer(pillarTB);
  //Procedural::BoxGenerator().setPosition(0,6.5,0).addToTriangleBuffer(pillarTB);
  Procedural::BoxGenerator().setPosition(0,.5,0).addToTriangleBuffer(pillarTB);
  pillarTB.transformToMesh("pillar");
  // We put the pillars on the side of the road
  //  for (int i=0;i<p.getSegCount();i++)
    //    if (i%2==0)
      
      // putMeshMat("pillar", "Examples/Marble", p.getPoint(i)+4*p.getAvgDirection(i).crossProduct(Ogre::Vector3::UNIT_Y).normalisedCopy());
  // -- Jarre
  // 
  Procedural::TriangleBuffer tb;
  // Body
  Procedural::Shape jarreShape = Procedural::CubicHermiteSpline2().addPoint(Ogre::Vector2(0,0), Ogre::Vector2::UNIT_X, Ogre::Vector2::UNIT_X)
    .addPoint(Ogre::Vector2(2,3))
    .addPoint(Ogre::Vector2(.5,5), Ogre::Vector2(-1,1).normalisedCopy(), Ogre::Vector2::UNIT_Y)
    .addPoint(Ogre::Vector2(1,7), Ogre::Vector2(1,1).normalisedCopy()).realizeShape().thicken(.1f).getShape(0);
  Procedural::Lathe().setShapeToExtrude(&jarreShape).addToTriangleBuffer(tb);
  // Handle 1
  Procedural::Shape jarreHandleShape = Procedural::CircleShape().setRadius(.2f).realizeShape();
  Procedural::Path jarreHandlePath = Procedural::CatmullRomSpline3().addPoint(Ogre::Vector3(0,6.5f,.75f))
    .addPoint(Ogre::Vector3(0,6,1.5f))
    .addPoint(Ogre::Vector3(0,5,.55f)).setNumSeg(10).realizePath();                 
  Procedural::Extruder().setShapeToExtrude(&jarreHandleShape).setExtrusionPath(&jarreHandlePath).addToTriangleBuffer(tb);     
  // Handle2
  jarreHandlePath.reflect(Ogre::Vector3::UNIT_Z);
  Procedural::Extruder().setShapeToExtrude(&jarreHandleShape).setExtrusionPath(&jarreHandlePath).addToTriangleBuffer(tb);     
  tb.transformToMesh("jarre");
  //  putMeshMat("jarre", "Examples/Marble", Vector3(5,0,5));

  Ogre::Entity *ent = mSceneMgr->createEntity("extrudeTest", "extrudedMesh");
  ent->setMaterialName("OgreBall/Transparent");
  Ogre::SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode("test");
  node->attachObject(ent);
  node->scale(500,500,500);

  //  MeshStrider *Strider = new MeshStrider(meshptr.get());
}

void OgreBallApplication::createCamera(void) {
  BaseApplication::createCamera();
  mCamera->setPosition(0,500,2000);
  mCamera->lookAt(0,0,0);
}

void OgreBallApplication::createFrameListener(void) {
  BaseApplication::createFrameListener();

  // Initialize CEGUI
  CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
  CEGUI::Font::setDefaultResourceGroup("Fonts");
  CEGUI::Scheme::setDefaultResourceGroup("Schemes");
  CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
  CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

  mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
  //  CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
  //  CEGUI::SchemeManager::getSingleton().create("WindowsLook.scheme");
}

//-------------------------------------------------------------------------------------
bool OgreBallApplication::frameStarted( const Ogre::FrameEvent &evt ) {
  bool result =  BaseApplication::frameStarted(evt);
  static Ogre::Real time = mTimer->getMilliseconds();

  Ogre::Real elapsedTime = mTimer->getMilliseconds() - time;
  time = mTimer->getMilliseconds();

  if (mPhysics) mPhysics->stepSimulation(elapsedTime);
  return result;
}

//-------------------------------------------------------------------------------------
bool OgreBallApplication::keyPressed( const OIS::KeyEvent &arg ) {
  return BaseApplication::keyPressed(arg);
}

bool OgreBallApplication::keyReleased( const OIS::KeyEvent &arg ) {
  return BaseApplication::keyReleased(arg);
}

bool OgreBallApplication::mouseMoved( const OIS::MouseEvent &arg ) {
  return BaseApplication::mouseMoved(arg);
}
bool OgreBallApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  return BaseApplication::mouseReleased(arg, id);
}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
  {
    // Create application object
    OgreBallApplication app;

    try {
      app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
      MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
      std::cerr << "An exception has occured: " <<
        e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
  }

#ifdef __cplusplus
}
#endif
