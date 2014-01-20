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
  mPhysics = new Physics();
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

void createTransform(Ogre::Entity* entity, Ogre::SceneNode* newNode, Physics* mPhysics) {
  //create the new shape, and tell the physics that is a Box
  btCollisionShape *newRigidShape = new btSphereShape(btScalar(125));
 
  //set the initial position and transform. For this demo, we set the tranform to be none
  btTransform startTransform;
  startTransform.setIdentity();
  startTransform.setRotation(btQuaternion(1.0f, 1.0f, 1.0f, 0));
 
  //set the mass of the object. a mass of "0" means that it is an immovable object
  btScalar mass = 0.0f;
  btVector3 localInertia(0,0,0);
 
  startTransform.setOrigin(btVector3(20,100,0));
  newRigidShape->calculateLocalInertia(mass, localInertia);
 
  //actually contruvc the body and add it to the dynamics world
  btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);
 
  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, newRigidShape, localInertia);
  btRigidBody *body = new btRigidBody(rbInfo);
  body->setRestitution(1);
  body->setUserPointer(newNode);
 
  mPhysics->getDynamicsWorld()->addRigidBody(body);
}

//-------------------------------------------------------------------------------------
void BallApp::createScene(void)
{
  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.05f, 0));
  mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

  // Balls
  Ogre::Entity* eBall = mSceneMgr->createEntity("Ball", "sphere.mesh");
  eBall->setMaterialName("Ogre/Eyes");
  eBall->setCastShadows(true);
  Ogre::SceneNode* ballNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("BallNode", Ogre::Vector3(0,100,0));
  ballNode->attachObject(eBall);

  createTransform(eBall, ballNode, mPhysics);

  // Environment

  Ogre::Plane lPlane(Ogre::Vector3::UNIT_X, 0);
  Ogre::Plane rPlane(Ogre::Vector3::NEGATIVE_UNIT_X, 0);
  Ogre::Plane bPlane(Ogre::Vector3::UNIT_Y, 0);
  Ogre::Plane tPlane(Ogre::Vector3::NEGATIVE_UNIT_Y, 0);
  Ogre::Plane farPlane(Ogre::Vector3::UNIT_Z, 0);
  Ogre::Plane nearPlane(Ogre::Vector3::NEGATIVE_UNIT_Z, 0);

  Ogre::MeshManager::getSingleton().createPlane("ground",
                                                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                bPlane, 1500, 1500, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);

  Ogre::Entity* eGround = mSceneMgr->createEntity("Ground", "ground");
  eGround->setMaterialName("Examples/Rockwall");
  eGround->setCastShadows(false);
  Ogre::SceneNode* groundNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("GroundNode");
  groundNode->attachObject(eGround);

  // Ground Physics
  btScalar groundMass(0.1);
  btVector3 localGroundInertia(0,0,0);
  btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.0), btScalar(50.0), btScalar(50.0)));

  btTransform groundTransform;
  groundTransform.setIdentity();
  groundTransform.setOrigin(btVector3(0, 0, 0));
  btDefaultMotionState *groundMotionState = new btDefaultMotionState(groundTransform);

  groundShape->calculateLocalInertia(groundMass, localGroundInertia);

  btRigidBody::btRigidBodyConstructionInfo
    groundRBInfo(groundMass, groundMotionState, groundShape, localGroundInertia);
  btRigidBody *groundBody = new btRigidBody(groundRBInfo);

  //add the body to the dynamics world
  mPhysics->getDynamicsWorld()->addRigidBody(groundBody);

  // Physics Test
  Ogre::Entity *entity = mSceneMgr->createEntity("testCubeEnt", "cube.mesh");
  Ogre::SceneNode *newNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("testNode");
  newNode->attachObject(entity);

  // Create the new shape, and tell the physics that is a Box
  btCollisionShape *newRigidShape = new btSphereShape(btScalar(50));

  //  btCollisionShape *newRigidShape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
  //  mPhysics->getCollisionShapes().push_back(newRigidShape);

  // Set the initial position and transform. For this demo, we set the tranform to be none
  btTransform startTransform;
  startTransform.setIdentity();
  startTransform.setRotation(btQuaternion(1.0f, 1.0f, 0.0f, 0));

  // Set the mass of the object. a mass of "0" means that it is an immovable object
  btScalar mass = 0.1f;
  btVector3 localInertia(0,0,0);

  startTransform.setOrigin(btVector3(0,300,0));
  newRigidShape->calculateLocalInertia(mass, localInertia);

  // Construct the body and add it to the dynamics world
  btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);

  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, newRigidShape, localInertia);
  btRigidBody *body = new btRigidBody(rbInfo);
  body->setRestitution(1);
  body->setUserPointer(newNode);

  mPhysics->getDynamicsWorld()->addRigidBody(body);
  
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
