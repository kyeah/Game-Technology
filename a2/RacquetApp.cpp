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
#include <btBulletDynamicsCommon.h>
#include "RacquetApp.h"
#include "RacquetObject.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <macUtils.h>
#   include "AppDelegate.h"
#endif

//-------------------------------------------------------------------------------------
RacquetApp::RacquetApp(void)
{
  mPhysics = new Physics(btVector3(0,0,0));
  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
}
//-------------------------------------------------------------------------------------
RacquetApp::~RacquetApp(void)
{
}

void RacquetApp::createCamera(void) {
  BaseApplication::createCamera();
  mCamera->setPosition(3500,-700,-3500);
  mCamera->lookAt(-500,-350,500);
}

bool RacquetApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  if(id == OIS::MB_Left) { 
   
    new Ball(mSceneMgr, 0, mPhysics, 
             btVector3(-500,-300,500), 
             btVector3( rand() % 120 - 60, 500, rand() % 80 - 40));
    
    mRacquet->translate(btVector3(100, 100, 100));

  } else if (id == OIS::MB_Right) {
    static int gravity = 1;
    btDiscreteDynamicsWorld *world = mPhysics->getDynamicsWorld();
    if (gravity == 0) {
      world->setGravity(btVector3(0,0,0));
      mDetailsPanel->setParamValue(3, "Off");
    } else if (gravity == 1) {
      world->setGravity(btVector3(0,980,0));
      mDetailsPanel->setParamValue(3, "Upward");
    } else if (gravity == 2) {
      world->setGravity(btVector3(0,-980,0));
      mDetailsPanel->setParamValue(3, "Downward");
    }

    gravity = (gravity+1)%3;
  }
  
  return BaseApplication::mouseReleased(arg, id);
}

//-------------------------------------------------------------------------------------
void RacquetApp::createScene(void)
{
  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.05f, 0));
  mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

  // Boxed Environment
  Ogre::Plane planes[] = {
    Ogre::Plane(Ogre::Vector3::UNIT_X, 0),
    Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_X, 0),
    Ogre::Plane(Ogre::Vector3::UNIT_Y, 0),
    Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Y, 0),
    Ogre::Plane(Ogre::Vector3::UNIT_Z, 0),
    Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Z, 0)
  };

  std::string pNames[] = {
    "leftWall", "rightWall", "ground", "ceiling", "farWall", "nearWall"
  };

  Ogre::Vector3 up[] = {
    Ogre::Vector3::UNIT_Y, Ogre::Vector3::UNIT_Y, 
    Ogre::Vector3::UNIT_Z, Ogre::Vector3::UNIT_Z, 
    Ogre::Vector3::UNIT_X, Ogre::Vector3::UNIT_X
  };
  
  int l,w,h;
  l = w = h = 3000;
  
  btVector3 pos[] = {
    btVector3(-w/2,0,0),
    btVector3(w/2,0,0),
    btVector3(0,-h/2,0),
    btVector3(0,h/2,0),
    btVector3(0,0,-l/2),
    btVector3(0,0,l/2),
  };
 
  for (int i = 0; i < 6; i++) {
    Ogre::MeshManager::getSingleton().createPlane(pNames[i],
                                                  Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  planes[i], l, w, 20, 20, true, 1, 5, 5, up[i]);

    Ogre::Entity* entity = mSceneMgr->createEntity(pNames[i], pNames[i]);
    if (pNames[i] == "ground") {
      entity->setMaterialName("Examples/Rockwall");
    }
    entity->setCastShadows(false);
    
    Ogre::SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode(pNames[i]);
    node->attachObject(entity);

    mPhysics->addRigidBox(entity, node, 0.0f, 0.95f, btVector3(0,0,0), pos[i]);  
  }
  
  // Lights
  Ogre::Light* pLight = mSceneMgr->createLight( "PointLight" );
  pLight->setType(Ogre::Light::LT_POINT);
  pLight->setPosition(-150,300,250);
  pLight->setDiffuseColour(1,0,0);
  pLight->setSpecularColour(1,0,0);

  Ogre::Light* dLight = mSceneMgr->createLight( "DirectionalLight" );
  dLight->setType(Ogre::Light::LT_DIRECTIONAL);
  dLight->setDiffuseColour(Ogre::ColourValue(.25,.25,0));
  dLight->setSpecularColour(Ogre::ColourValue(.25,.25,0));
  dLight->setDirection(Ogre::Vector3(0,-1,1));

  Ogre::Light* sLight = mSceneMgr->createLight("SpotLight");
  sLight->setType(Ogre::Light::LT_SPOTLIGHT);
  sLight->setDiffuseColour(0, 0, 1);
  sLight->setSpecularColour(0, 0, 1);

  sLight->setDirection(-1, -1, 0);
  sLight->setPosition(Ogre::Vector3(0, 400, 200));

  sLight->setSpotlightRange(Ogre::Degree(35), Ogre::Degree(50));

  mRacquet = new Racquet(mSceneMgr, 0, mPhysics,
              btVector3(100, 100, 50));

  for (int i = 0; i < 4; i++)
    new Ball(mSceneMgr, 0, mPhysics,
             btVector3(-500, -300, 500),
             btVector3(rand() % 800 - 400, rand() % 800 - 400, rand() % 800 - 400));
}

bool RacquetApp::frameStarted(const Ogre::FrameEvent &evt) {
  bool result = BaseApplication::frameStarted(evt);
  static Ogre::Real time = mRoot->getTimer()->getMilliseconds();

  Ogre::Real elapsedTime = mRoot->getTimer()->getMilliseconds() - time;
  time = mRoot->getTimer()->getMilliseconds();  
  if (mPhysics != NULL) {
    mPhysics->stepSimulation(elapsedTime);
  }

  return result;
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
#if defined(OGRE_IS_IOS)
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
    [pool release];
    return retVal;
#elif (OGRE_PLATFORM == OGRE_PLATFORM_APPLE) && __LP64__
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    mAppDelegate = [[AppDelegate alloc] init];
    [[NSApplication sharedApplication] setDelegate:mAppDelegate];
    int retVal = NSApplicationMain(argc, (const char **) argv);

    [pool release];

    return retVal;
#else
    // Create application object
    RacquetApp app;

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
#endif
    return 0;
  }

#ifdef __cplusplus
}
#endif
