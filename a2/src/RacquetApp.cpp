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

const static int DETAILS_HIGHSCORE = 0;
const static int DETAILS_SCORE = 1;
const static int DETAILS_GRAVITY = 3;

int highscore = 0;
int score = 0;

//-------------------------------------------------------------------------------------
RacquetApp::RacquetApp(void)
{
  mPhysics = new Physics(btVector3(0,-980,0));
  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
  mDirection = btVector3(0, 0, 0);
  oDirection = Ogre::Vector3(0, 0, 0);
}
//-------------------------------------------------------------------------------------
RacquetApp::~RacquetApp(void)
{
}

void RacquetApp::createCamera(void) {
  BaseApplication::createCamera();
  mCamera->setPosition(-1200,0,-7000);
  mCamera->lookAt(0,0,500);
}

void RacquetApp::createFrameListener(void) {
  BaseApplication::createFrameListener();

  Ogre::StringVector items;
  items.push_back("Highscore");
  items.push_back("Current Score");
  items.push_back("");
  items.push_back("Gravity");
  
  mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
  mDetailsPanel->setParamValue(DETAILS_HIGHSCORE, "0");
  mDetailsPanel->setParamValue(DETAILS_SCORE, "0");
  mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Downwards");
}

bool RacquetApp::keyPressed( const OIS::KeyEvent &arg ) {
  switch(arg.key){
    case OIS::KC_D:
      mDirection += btVector3(-40, 0, 0);
      oDirection.x += -40;
      break;
    case OIS::KC_S:
      mDirection += btVector3(0, 0, -40);
      oDirection.z += -40;
      break;
    case OIS::KC_A:
      mDirection += btVector3(40, 0, 0);
      oDirection.x += 40;
      break;
    case OIS::KC_W:
      mDirection += btVector3(0, 0, 40);
      oDirection.z += 40;
      break;
  }

  return BaseApplication::keyPressed(arg);
}

bool RacquetApp::keyReleased(const OIS::KeyEvent &arg){
  switch(arg.key){
    case OIS::KC_J:
      case OIS::KC_D:
      mDirection -= btVector3(-40, 0, 0);
      oDirection.x -= -40;
      break;
    case OIS::KC_S:
      mDirection -= btVector3(0, 0, -40);
      oDirection.z -= -40;
      break;
    case OIS::KC_A:
      mDirection -= btVector3(40, 0, 0);
      oDirection.x -= 40;
      break;
    case OIS::KC_W:
      mDirection -= btVector3(0, 0, 40);
      oDirection.z -= 40;
      break;


  }

  return BaseApplication::keyPressed(arg);
}

bool RacquetApp::mouseMoved( const OIS::MouseEvent& arg ) {
  int x = -arg.state.X.rel;
  int y = -arg.state.Y.rel;
  int z = -arg.state.Z.rel;
  
  mRacquet->translate(btVector3(x,y,z));
}

bool RacquetApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  if(id == OIS::MB_Left) { 
    
    static int ballID;
    std::stringstream ss;
    ss << "myBall" << ballID;
    std::string ent = ss.str();
    ss << "node";
    ballID++;
    
    Ball *m = new Ball(mSceneMgr, ent, ss.str(), 0, mPhysics, 
                       btVector3(-500,-300,500), 
                       btVector3( rand() % 120 - 60, 500, rand() % 80 - 40));
    
  } else if (id == OIS::MB_Right) {
    static int gravity = 0;
    btDiscreteDynamicsWorld *world = mPhysics->getDynamicsWorld();
    if (gravity == 0) {
      world->setGravity(btVector3(0,0,0));
      mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Off");
    } else if (gravity == 1) {
      world->setGravity(btVector3(0,980,0));
      mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Upward");
    } else if (gravity == 2) {
      world->setGravity(btVector3(0,-980,0));
      mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Downward");
    }

    gravity = (gravity+1)%3;
  }
  
  return BaseApplication::mouseReleased(arg, id);
}

//-------------------------------------------------------------------------------------
void RacquetApp::createScene(void)
{
  mSceneMgr->setAmbientLight(Ogre::ColourValue(.5f, .5f, .5f));
  mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

  // Boxed Environment
  Ogre::Plane planes[] = {
    Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_X, 0),
    Ogre::Plane(Ogre::Vector3::UNIT_X, 0),
    Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Y, 0),
    Ogre::Plane(Ogre::Vector3::UNIT_Y, 0),
    Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Z, 0),
    Ogre::Plane(Ogre::Vector3::UNIT_Z, 0)
  };

  std::string pNames[] = {
    "leftWall", "rightWall", "ground", "ceiling", "farWall", "nearWall"
  };

  Ogre::Vector3 up[] = {
    Ogre::Vector3::UNIT_Y, Ogre::Vector3::UNIT_Y,
    Ogre::Vector3::UNIT_Z, Ogre::Vector3::UNIT_Z, 
    Ogre::Vector3::UNIT_X, Ogre::Vector3::UNIT_X
  };
  
  //w applies to leftWall/rightWall
  //h applies to ground/ceiling
  //l applies to farWall/nearWall
  int l,w,h;
  l = 7500;
  w = h = 4500;
  
  btVector3 pos[] = {
    btVector3(-w/2,0,0),
    btVector3(w/2,0,0),
    btVector3(0,-h/2,0),
    btVector3(0,h/2,0),
    btVector3(0,0,l/2),
    btVector3(0,0,-l/2)
  };
 
  int width, height;
  width = w;
  height = h;
  for (int i = 0; i < 6; i++) {
    if(pNames[i] == "leftWall" || pNames[i] == "rightWall") { width = l; height = w; }
    if(pNames[i] == "ground" || pNames[i] == "ceiling") { width = w; height = l;}
    if(pNames[i] == "farWall" || pNames[i] == "nearWall") { width = w; height = w;}
    Ogre::MeshManager::getSingleton().createPlane(pNames[i],
                                                  Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  planes[i], width, height, 20, 20, true, 1, 5, 5, up[i]);

    Plane *p = new Plane(mSceneMgr, pNames[i], pNames[i], pNames[i], 0, mPhysics, pos[i]);
    
    if (pNames[i] == "ground") {
      p->getEntity()->setMaterialName("Court/Floor");
    } 
  }
  
  // Lights
  Ogre::Light* lights[9];
  int z;
  for(z = 0; z < 9; z++) { 
	lights[z] = mSceneMgr->createLight("point light" + z); 
  	lights[z]->setType(Ogre::Light::LT_POINT);
	lights[z]->setDiffuseColour(.1,.1,.1);
	lights[z]->setSpecularColour(.1,.1,.1);	
  }
  lights[0]->setPosition(-1499,1499,0);
  lights[1]->setPosition(-1499,1499,1000);
  lights[2]->setPosition(-1499,1499,2000);
  lights[3]->setPosition(-1000,1499,2499);
  lights[4]->setPosition(0,1499,2499);
  lights[5]->setPosition(1000,1499,2499);
  lights[6]->setPosition(1499,1499,2000);
  lights[7]->setPosition(1499,1499,1000);
  lights[8]->setPosition(1499,1499,0);

  mRacquet = new Racquet(mSceneMgr, "Racquet", "Racquetnode", 0, mPhysics,
                         btVector3(100, 100, -3500));


  mBall = new Ball(mSceneMgr, "Ball", "BallNode", 0, mPhysics, 
                   btVector3(100,100,150), 
                   btVector3( rand() % 120 - 60, rand() % 80 - 40, 5000),
                   1000);
}

bool RacquetApp::frameStarted(const Ogre::FrameEvent &evt) {
  bool result = BaseApplication::frameStarted(evt);
  static Ogre::Real time = mTimer->getMilliseconds();

  Ogre::Real elapsedTime = mTimer->getMilliseconds() - time;
  time = mTimer->getMilliseconds();  

  if (mPhysics != NULL) {
    mPhysics->stepSimulation(elapsedTime);
  }

  mRacquet->getBody()->translate(mDirection);
  mRacquet->translate(mDirection);


  mDetailsPanel->setParamValue(DETAILS_SCORE, std::to_string(score));  
  if (score > highscore) {
    highscore = score;
    mDetailsPanel->setParamValue(DETAILS_HIGHSCORE, std::to_string(score));
  }

  btVector3 pos = mBall->getPosition();
  mCamera->lookAt(pos[0], pos[1], pos[2]);

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
