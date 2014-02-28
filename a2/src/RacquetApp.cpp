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
#include "Sounds.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <macUtils.h>
#   include "AppDelegate.h"
#endif

const static int DETAILS_HIGHSCORE = 0;
const static int DETAILS_LASTSCORE = 2;
const static int DETAILS_SCORE = 3;
const static int DETAILS_GRAVITY = 4;

const static int SWING_DELAY = 5;
const static int UNSWING_DELAY = 10;

int highscore = 0;
int lastscore = 0;
int score = 0;

btVector3 racquetInitPos(0,700.0f,0);
btVector3 playerInitPos(100,-1200,-2245);
btVector3 *axis; // Swing rotation axis
static int gravMag = 7000;
static bool pongMode = false;
static bool right_mouse_button = false;
Ogre::Light* discolights[6];

RacquetApp *instance;

//-------------------------------------------------------------------------------------
RacquetApp::RacquetApp(void)
{
  mPhysics = new Physics(btVector3(0,-gravMag,0));
  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
  mDirection = btVector3(0, 0, 0);
  oDirection = Ogre::Vector3(0, 0, 0);
  MAX_SPEED = btScalar(8000);
  swing = 0;
  unswing = 0;
  movementSpeed = 1;
  Sounds::init();
}
//-------------------------------------------------------------------------------------
RacquetApp::~RacquetApp(void)
{
}

void RacquetApp::createCamera(void) {
  BaseApplication::createCamera();
  mCamera->setPosition(0,0,-7000);
  mCamera->lookAt(0,0,500);
}

void RacquetApp::createFrameListener(void) {
  BaseApplication::createFrameListener();

  Ogre::StringVector items;
  items.push_back("Highscore");
  items.push_back("");
  items.push_back("Last Score");
  items.push_back("Current Score");
  items.push_back("Gravity");

  mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
  mDetailsPanel->setParamValue(DETAILS_HIGHSCORE, "0");
  mDetailsPanel->setParamValue(DETAILS_LASTSCORE, "0");
  mDetailsPanel->setParamValue(DETAILS_SCORE, "0");
  mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Downwards");
}

bool RacquetApp::keyPressed( const OIS::KeyEvent &arg ) {
  static bool vert = false;

  switch(arg.key){
  case OIS::KC_D:
    mDirection += btVector3(-40, 0, 0);
    oDirection.x += -40;
    return true;
  case OIS::KC_S:
    if (vert) {
      mDirection += btVector3(0, -40, 0);
      oDirection.y += -40;
    } else {
      mDirection += btVector3(0, 0, -40);
      oDirection.z += -40;
    }
    return true;
  case OIS::KC_A:
    mDirection += btVector3(40, 0, 0);
    oDirection.x += 40;
    return true;
  case OIS::KC_W:
    if (vert) {
      mDirection += btVector3(0, 40, 0);
      oDirection.y += 40;
    } else {
        mDirection += btVector3(0, 0, 40);
        oDirection.z += 40;
    }

    return true;
  case OIS::KC_LSHIFT:
    movementSpeed = 2;
    return true;
  case OIS::KC_SPACE:
    if(swing == 0 && unswing == 0) {
      swing = SWING_DELAY;
      Sounds::playSound(Sounds::RACQUET_SWOOSH, 100);
    }
    return true;
  }

  return BaseApplication::keyPressed(arg);
}

void RacquetApp::restart() {
  static int gamenum = 0;
  mPhysics->removeObject(mBall);
  mBall->setPosition(btVector3(0,0,0));
  mBall->setVelocity(btVector3(0,0,0));
  mBall->updateTransform();
  mBall->addToSimulator();
  std::cout << "Game " << gamenum++ << ": " << score << std::endl;
  lastscore = score;
  score = 0;
}

bool RacquetApp::keyReleased(const OIS::KeyEvent &arg){
  static bool vert = false;

  switch(arg.key){
  case OIS::KC_R:
    restart();
    return true;
  case OIS::KC_P:
    swing = unswing = 0;
    pongMode = !pongMode;
    mPlayer->setOrientation(btQuaternion(0,0,0,1));
    mPlayer->setPosition(playerInitPos);
    mPlayer->getEntity()->setVisible(!mPlayer->getEntity()->isVisible());
    return true;
  case OIS::KC_J:
  case OIS::KC_D:
    mDirection -= btVector3(-40, 0, 0);
    oDirection.x -= -40;
    return true;
  case OIS::KC_S:
    if (vert) {
      mDirection -= btVector3(0, -40, 0);
      oDirection.y -= -40;
    } else {
      mDirection -= btVector3(0, 0, -40);
      oDirection.z -= -40;
    }
    return true;
  case OIS::KC_A:
    mDirection -= btVector3(40, 0, 0);
    oDirection.x -= 40;
    return true;
  case OIS::KC_W:
    if (vert) {
      mDirection -= btVector3(0, 40, 0);
      oDirection.y -= 40;
    } else {
        mDirection -= btVector3(0, 0, 40);
        oDirection.z -= 40;
    }
    return true;
  case OIS::KC_LSHIFT:
    movementSpeed = 1;
    return true;
  case OIS::KC_G:
    static int gravity = 0;
    btDiscreteDynamicsWorld *world = mPhysics->getDynamicsWorld();
    if (gravity == 0) {
      world->setGravity(btVector3(0,0,0));
      mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Off");
    } else if (gravity == 1) {
      world->setGravity(btVector3(0,gravMag,0));
      mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Upward");
    } else if (gravity == 2) {
      world->setGravity(btVector3(0,-gravMag,0));
      mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Downward");
    }

    gravity = (gravity+1)%3;
    return true;
  }

  return BaseApplication::keyPressed(arg);
}

bool RacquetApp::mouseMoved( const OIS::MouseEvent& arg ) {
  if (swing == 0 && unswing == 0) {
    int x = arg.state.X.rel;
    int y = arg.state.Y.rel;

    static float rotfactor = 6.28 / 1800;
    
    if (pongMode) {
      //Boundaries
      if(x < 0 && mRacquet->getPosition().getX() >= 2000){
        x = 0;
      }
      if(x > 0 && mRacquet->getPosition().getX() <= -2000){
        x = 0;
      }
      if(y < 0 && mRacquet->getPosition().getY() >= 2000){
        y = 0;
      }
      if(y > 0 && mRacquet->getPosition().getY() <= -2000){
        y = 0;
      }



      mPlayer->translate(btVector3(-x,-y,0));
    } else {
      mPlayer->rotate(btQuaternion(btVector3(0,0,1), btScalar(x*rotfactor)));
    }
  }
}

bool RacquetApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  if(swing == 0 && unswing == 0) {
    
    if(id == OIS::MB_Left || id == OIS::MB_Right) {
      swing = SWING_DELAY;
      Sounds::playSound(Sounds::RACQUET_SWOOSH, 100);

      Ogre::Vector3 p = mRacquet->getNode()->getPosition();
      axis = new btVector3(p[1], -p[0], 0);

      right_mouse_button = (id == OIS::MB_Right);
    }
  }
  
  return BaseApplication::mouseReleased(arg, id);
}

void RacquetApp::createNewScoringPlane(int points, btVector3 pos, btVector3 speed, btVector3 linearFactor, btVector3 angularFactor) {
  static int wallID;
  std::stringstream ss;
  ss << points << "wall";
  std::string mesh = ss.str();
  ss << wallID;
  std::string ent = ss.str();
  ss << "node";
  std::string node = ss.str();
  wallID++;

  ScoringPlane *extra = new ScoringPlane(worldWidth, worldLength, worldHeight,
                                         mSceneMgr, ent, mesh, node, 0, mPhysics,
                                         pos, speed, 0.0, 1.0);

  extra->points = points;

  extra->cycleColor();
  extra->getBody()->setLinearFactor(linearFactor);
  extra->getBody()->setAngularFactor(angularFactor);
}

//-------------------------------------------------------------------------------------
void RacquetApp::createScene(void)
{
  mSceneMgr->setAmbientLight(Ogre::ColourValue(.5f, .5f, .5f));
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
    "leftWall", "rightWall", "ground", "ceiling", "nearWall", "farWall"
  };

  Ogre::Vector3 up[] = {
    Ogre::Vector3::UNIT_Y, Ogre::Vector3::UNIT_Y,
    Ogre::Vector3::UNIT_Z, Ogre::Vector3::UNIT_Z,
    Ogre::Vector3::UNIT_X, Ogre::Vector3::UNIT_X
  };

  //w applies to leftWall/rightWall
  //h applies to ground/ceiling
  //l applies to farWall/nearWall
  int w, l, h;
  worldLength = l = 5000;
  worldWidth = worldHeight = w = h = 4500;

  btVector3 pos[] = {
    btVector3(-w/2,0,0),
    btVector3(w/2,0,0),
    btVector3(0,-h/2,0),
    btVector3(0,h/2,0),
    btVector3(0,0,-l/2),
    btVector3(0,0,l/2)
  };

  //lines
/*  Ogre::Entity* line1 = mSceneMgr->createEntity("L1", "cube.mesh");
  Ogre::Entity* line2 = mSceneMgr->createEntity("L2", "cube.mesh");
  Ogre::Entity* line3 = mSceneMgr->createEntity("L3", "cube.mesh");
  Ogre::Entity* line4 = mSceneMgr->createEntity("L4", "cube.mesh");

  Ogre::SceneNode* n1 = mSceneMgr->getRootSceneNode()->createChildSceneNode("N1",Ogre::Vector3(0.0f, 2245.0f, -1500.0f));
  Ogre::SceneNode* n2 = mSceneMgr->getRootSceneNode()->createChildSceneNode("N2",Ogre::Vector3(0.0f, -2245.0f, -1500.0f));
  Ogre::SceneNode* n3 = mSceneMgr->getRootSceneNode()->createChildSceneNode("N3",Ogre::Vector3(2245.0f, 0.0f, -1500.0f));
  Ogre::SceneNode* n4 = mSceneMgr->getRootSceneNode()->createChildSceneNode("N4",Ogre::Vector3(-2245.0f, 0.0f, -1500.0f));

  n1->attachObject(line1); n1->scale(300, .5, .5);
  n2->attachObject(line2); n2->scale(300, .5, .5);
  n3->attachObject(line3); n3->scale(.5, 300, .5);
  n4->attachObject(line4); n4->scale(.5, 300, .5);
*/
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
    } else {
      p->getEntity()->setMaterialName("Court/Wall");
    }

    if (pNames[i] == "farWall") p->points = 1;
  }

  Ogre::MeshManager::getSingleton().createPlane("2wall",
                                                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                planes[5], w/4, w/4, 20, 20, true, 1, 5, 5, up[5]);

  Ogre::MeshManager::getSingleton().createPlane("4wall",
                                                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                planes[5], w/6, w/6, 20, 20, true, 1, 5, 5, up[5]);

  // Lights
  Ogre::Light* lights[9];
  int z;
  for(z = 0; z < 9; z++) {
    std::stringstream ss;
    ss << "point light" << z;
    lights[z] = mSceneMgr->createLight(ss.str());
    lights[z]->setType(Ogre::Light::LT_POINT);
    lights[z]->setDiffuseColour(.1,.1,.1);
    lights[z]->setSpecularColour(.1,.1,.1);
    
    ss << z;
    if (z < 6) {
      discolights[z] = mSceneMgr->createLight(ss.str());
      discolights[z]->setType(Ogre::Light::LT_SPOTLIGHT);
      discolights[z]->setDiffuseColour(rand(),rand(),rand());
      discolights[z]->setSpecularColour(rand(),rand(),rand());
      discolights[z]->setDirection(rand(), rand(), rand());
      discolights[z]->setSpotlightOuterAngle(Ogre::Radian(0.1));
    }
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

  // Front Wall
  discolights[0]->setDirection(-0.5,0,1);
  discolights[1]->setDirection(0.5,0,1);
  discolights[0]->setPosition(0,1800,0);
  discolights[1]->setPosition(0,1800,0);

  // Right Wall
  discolights[2]->setDirection(-1,0,0);
  discolights[3]->setDirection(-1,0,-1);
  discolights[2]->setPosition(0,-1800,0);
  discolights[3]->setPosition(0,-1800,0);
  
  // Left Wall
  discolights[4]->setDirection(1,0,0);
  discolights[5]->setDirection(1,0,-1);
  discolights[4]->setPosition(0,-1800,0);
  discolights[5]->setPosition(0,-1800,0);

  mPlayer = new Dude(mSceneMgr, "Player", "PlayerNode", 0, mPhysics,
                     playerInitPos, btVector3(0,0,0), 0);
  
  mRacquet = new Racquet(mSceneMgr, "Racquet", "Racquetnode", mPlayer->getNode(), mPhysics,
                         racquetInitPos);

  mBall = new Ball(mSceneMgr, "Ball", "BallNode", 0, mPhysics,
                   btVector3(100,100,150),
                   btVector3( rand() % 120 - 60, rand() % 80 - 40, 6000),
                   1000);

  mBall->getNode()->attachObject(mSceneMgr->createParticleSystem("fountain1", "Examples/PurpleFountain"));
  mBall->getNode()->attachObject(mSceneMgr->createParticleSystem("fountain2", "Examples/PurpleFountain"));

  if (pongMode) mPlayer->getEntity()->setVisible(false);

  createNewScoringPlane(2, btVector3( 0, rand() % 3500 - 2000, 5000/2 - 5));
  createNewScoringPlane(4, btVector3( 0, rand() % 3500 - 2000, 5000/2 - 5), btVector3(30,0,0));
}

bool RacquetApp::frameStarted(const Ogre::FrameEvent &evt) {
  for (int i = 0; i < 2; i++) {
    Ogre::Vector3 v = discolights[i]->getDirection();
    double x = v[0] + 0.02;
    if (x > 1.0) x = x - 2.0;
    discolights[i]->setDirection(x, v[1], v[2]);
  }
  for (int i = 2; i < 6; i++) {
    Ogre::Vector3 v = discolights[i]->getDirection();
    double z = v[2] + 0.02;
    if (z > 1.0) z = z - 2.0;
    discolights[i]->setDirection(v[0], v[1], z);
  }

  bool result = BaseApplication::frameStarted(evt);
  static Ogre::Real time = mTimer->getMilliseconds();

  Ogre::Real elapsedTime = mTimer->getMilliseconds() - time;
  time = mTimer->getMilliseconds();

  // Limit Ball Velocity
  ballVelocity = mBall->getBody()->getLinearVelocity();
  btScalar speed = ballVelocity.length();
  if(speed > MAX_SPEED){
    ballVelocity *= MAX_SPEED/speed;
    mBall->getBody()->setLinearVelocity(ballVelocity);
  }

  // Physics Simulation
  if (mPhysics != NULL) {
    mPhysics->stepSimulation(elapsedTime);
  }
  
  //store original vectors
  int oldZ = mDirection.getZ();
  int oldX = mDirection.getX();
  int oldY = mDirection.getY();

  //boundaries
  if(mRacquet->getPosition().getZ() >= 600){
    mDirection.setZ(-10);
  }
  if(mRacquet->getPosition().getZ() <= -2400){
    mDirection.setZ(10);
  }
  if(mRacquet->getPosition().getX() >= 2000){
    mDirection.setX(-10);
  }
  if(mRacquet->getPosition().getX() <= -2000){
    mDirection.setX(10);
  }
  if(mRacquet->getPosition().getY() >= 2000){
    mDirection.setY(-10);
  }
  if(mRacquet->getPosition().getY() <= -2000){
    mDirection.setY(10);
  }



  mPlayer->getBody()->translate(mDirection*movementSpeed);
  mPlayer->translate(mDirection*movementSpeed);

  //reset the vector after translation
  mDirection.setZ(oldZ);
  mDirection.setX(oldX);
  mDirection.setY(oldY);

  
  // Swings
  if(unswing > 0){
    if (pongMode || right_mouse_button) {
      mPlayer->translate(btVector3(0, 0, -25));
    } else if (axis) {
      mPlayer->rotate(btQuaternion(*axis, btScalar(-0.1)));
    }
    unswing--;
  }

  if(swing > 0){
    if (pongMode || right_mouse_button) {
      mPlayer->translate(btVector3(0, 0, 50));
    } else if (axis) {
      mPlayer->rotate(btQuaternion(*axis, btScalar(0.2)));
    }
    swing--;
    if(swing == 0)
      unswing = UNSWING_DELAY;
  }

  mDetailsPanel->setParamValue(DETAILS_LASTSCORE, std::to_string(lastscore));
  mDetailsPanel->setParamValue(DETAILS_SCORE, std::to_string(score));
  if (score > highscore) {
    highscore = score;
    mDetailsPanel->setParamValue(DETAILS_HIGHSCORE, std::to_string(highscore));
  }

  btVector3 pos = (pongMode ? mRacquet->getPosition() : mPlayer->getPosition());
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
    instance = &app;

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
