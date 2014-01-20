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
#include "BallApp.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <macUtils.h>
#   include "AppDelegate.h"
#endif

//-------------------------------------------------------------------------------------
BallApp::BallApp(void)
{
  mPhysics = new Physics(btVector3(0,-490,0));
  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
}
//-------------------------------------------------------------------------------------
BallApp::~BallApp(void)
{
}

void BallApp::createCamera(void) {
  BaseApplication::createCamera();
  mCamera->setPosition(0,100,900);
}

void BallApp::createBox(std::string entName, std::string nodeName, int x, int y, int z) {
  Ogre::Entity *entity = mSceneMgr->createEntity(entName, "sphere.mesh");
  entity->setCastShadows(true);
  
  Ogre::SceneNode *newNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(nodeName);
  newNode->attachObject(entity);

  mPhysics->addRigidSphere(entity, newNode, 0.1f, 1.0f, btVector3(0,0,0), btVector3(x,y,z), new btQuaternion(1.0f, 1.0f, 0, 0))->setLinearVelocity(btVector3(rand() % 120 - 60, 500, rand() % 80 - 40));
}

bool BallApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  static int boxID;
  
  if(id == OIS::MB_Left) {
    std::stringstream ss;
    ss << "myBox" << boxID;
    std::string ent = ss.str();
    ss << "node";
    createBox(ent, ss.str(), 20, 400, 0);
    boxID++;
  }
  
  return BaseApplication::mouseReleased(arg, id);
}

//-------------------------------------------------------------------------------------
void BallApp::createScene(void)
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

  for (int i = 0; i < 6; i++) {
    Ogre::MeshManager::getSingleton().createPlane(pNames[i],
                                                  Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  planes[i], 1500, 1500, 20, 20, true, 1, 5, 5, up[i]);

    Ogre::Entity* entity = mSceneMgr->createEntity(pNames[i], pNames[i]);
    if (pNames[i] == "ground") {
      entity->setMaterialName("Examples/Rockwall");
    }
    entity->setCastShadows(false);
    
    Ogre::SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode(pNames[i]);
    node->attachObject(entity);
    
    btVector3 pos[] = {
      btVector3(-750,0,0),
      btVector3(750,0,0),
      btVector3(0,-750,0),
      btVector3(0,750,0),
      btVector3(0,0,-750),
      btVector3(0,0,750),
    };
    
    mPhysics->addRigidBox(entity, node, 0.0f, 0.3f, btVector3(0,0,0), pos[i]);  
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
  sLight->setDiffuseColour(0, 0, 1.0);
  sLight->setSpecularColour(0, 0, 1.0);

  sLight->setDirection(-1, -1, 0);
  sLight->setPosition(Ogre::Vector3(0, 400, 200));

  sLight->setSpotlightRange(Ogre::Degree(35), Ogre::Degree(50));
}

bool BallApp::frameStarted(const Ogre::FrameEvent &evt) {
  bool result = BaseApplication::frameStarted(evt);
  if (mPhysics != NULL) {
    btDiscreteDynamicsWorld* world = mPhysics->getDynamicsWorld();
    world->stepSimulation(1.0f/60.0f);

    btAlignedObjectArray<btCollisionObject*> objs = world->getCollisionObjectArray();
    for (int i = 0; i < objs.size(); i++) {
      btCollisionObject *obj = objs[i];
      btRigidBody *body = btRigidBody::upcast(obj);
      
      if (body && body->getMotionState()) {
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);
        
        void *userPointer = body->getUserPointer();
        if (userPointer) {
          btQuaternion orientation = trans.getRotation();
          Ogre::SceneNode *sceneNode = static_cast<Ogre::SceneNode *>(userPointer);
          sceneNode->setPosition(Ogre::Vector3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()));
          sceneNode->setOrientation(Ogre::Quaternion(orientation.getW(), orientation.getX(), orientation.getY(), orientation.getZ()));
        }
      }
    }
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
    BallApp app;

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
