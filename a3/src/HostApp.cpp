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
#include "HostApp.h"
#include "RacquetObject.h"
#include "Sounds.h"
#include "SDL_net.h"
#include "common.h"
#include "Networking.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <macUtils.h>
#   include "AppDelegate.h"
#endif

#include <iostream>
#include <string>

//-------------------------------------------------------------------------------------
HostApp::HostApp(void)
{
  mPhysics = new Physics(btVector3(0,-gravMag,0));
  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
  MAX_SPEED = btScalar(8000);
  Sounds::init();
  connected = false;
  HostApp::Connect();
}
//-------------------------------------------------------------------------------------
HostApp::~HostApp(void)
{
  HostApp::Close();
}

void HostApp::createCamera(void) {
  BaseApplication::createCamera();
  mCamera->setPosition(0,0,-7000);
  mCamera->lookAt(0,0,500);
}

void HostApp::createFrameListener(void) {
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

bool HostApp::keyPressed( const OIS::KeyEvent &arg ) {
  return handleKeyPressed(arg.key, myId);
}

bool HostApp::handleKeyPressed( OIS::KeyCode arg, int userId ) {
  static bool vert = false;

  Player *mPlayer = findPlayer(userId);
  if (!mPlayer) return false;

  switch(arg){
  case OIS::KC_D:
    mPlayer->mDirection += btVector3(-40, 0, 0);
    mPlayer->oDirection.x += -40;
    return true;
  case OIS::KC_S:
    if (vert) {
      mPlayer->mDirection += btVector3(0, -40, 0);
      mPlayer->oDirection.y += -40;
    } else {
      mPlayer->mDirection += btVector3(0, 0, -40);
      mPlayer->oDirection.z += -40;
    }
    return true;
  case OIS::KC_A:
    mPlayer->mDirection += btVector3(40, 0, 0);
    mPlayer->oDirection.x += 40;
    return true;
  case OIS::KC_W:
    if (vert) {
      mPlayer->mDirection += btVector3(0, 40, 0);
      mPlayer->oDirection.y += 40;
    } else {
      mPlayer->mDirection += btVector3(0, 0, 40);
      mPlayer->oDirection.z += 40;
    }

    return true;
  case OIS::KC_LSHIFT:
    mPlayer->movementSpeed = 2;
    return true;
  case OIS::KC_SPACE:
    if(mPlayer->swing == 0 && mPlayer->unswing == 0) {
      mPlayer->swing = SWING_DELAY;
      Sounds::playSound(Sounds::RACQUET_SWOOSH, 100);
    }
    return true;
  }

  return false;
}

void HostApp::restart() {
  /*  static int gamenum = 0;
  mPhysics->removeObject(mBall);
  mBall->setPosition(btVector3(0,0,0));
  mBall->setVelocity(btVector3(0,0,0));
  mBall->updateTransform();
  mBall->addToSimulator();
  std::cout << "Game " << gamenum++ << ": " << score << std::endl;
  lastscore = score;
  score = 0;*/
}

void HostApp::Connect(){
  printf("in connect\nPlease enter your hostname (default: pastamancer.cs.utexas.edu):");
  std::string host;
  getline(std::cin, host);
  if (host.length() == 0)
    host = std::string("pastamancer.cs.utexas.edu");

  int port = 65501;

  printf("trying to connect to player 2...\n");
  SDLNet_Init();
  if(SDLNet_ResolveHost(&ip, host.c_str(), port) == -1){
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(0);
  }

  sd =SDLNet_TCP_Open(&ip);
  while(!sd){
    sd = SDLNet_TCP_Open(&ip);
    if(!sd){
      printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
      printf("trying again...\n");
    }
  }
  connected = true;
}

void HostApp::Send(char *msg, int len){
  if(connected){
    printf("sending, %s\n", msg);
    int result = SDLNet_TCP_Send(sd, (void*)msg, len);
    if(result < len)
      printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
  }
}

void HostApp::Close(){
  SDLNet_TCP_Close(csd);
  SDLNet_TCP_Close(sd);
  SDLNet_Quit();
  connected = false;
}

Player* HostApp::findPlayer(int userID) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (players[i] && players[i]->getId() == userID) {
      return players[i];
    }
  }
  return NULL;
}

Player* HostApp::addPlayer(int userID) {
  Player* mPlayer = new Player(userID);

  std::stringstream ss;
  ss << "Player" << userID;
  std::string playerEnt = ss.str();
  ss << "node";

  std::stringstream ssr;
  ssr << "Racquet" << userID;
  std::string racquetEnt = ssr.str();
  ssr << "node";

  mPlayer->setNode(new Dude(mSceneMgr, playerEnt, ss.str(), 0, mPhysics,
                            playerInitialPositions[userID], btVector3(0,0,0), 0));
  
  mPlayer->setRacquet(new Racquet(mSceneMgr, racquetEnt, ssr.str(), mPlayer->getNode()->getNode(), mPhysics,
                                  racquetInitPos));

  players[userID] = mPlayer;
  return mPlayer;
}

bool HostApp::keyReleased(const OIS::KeyEvent &arg){
  handleKeyReleased(arg.key, myId);
}

bool HostApp::handleKeyReleased(OIS::KeyCode arg, int userID) {
  static bool vert = false;

  Player *mPlayer = findPlayer(userID);
  if (!mPlayer) return false;

  switch(arg){
  case OIS::KC_R:
    restart();
    return true;
  case OIS::KC_P:
    mPlayer->swing = mPlayer->unswing = 0;
    mPlayer->pongMode = !mPlayer->pongMode;
    mPlayer->getNode()->setOrientation(btQuaternion(0,0,0,1));
    mPlayer->getNode()->setPosition(playerInitPos);
    mPlayer->getNode()->getEntity()->setVisible(!mPlayer->getNode()->getEntity()->isVisible());
    return true;
  case OIS::KC_J:
  case OIS::KC_D:
    mPlayer->mDirection -= btVector3(-40, 0, 0);
    mPlayer->oDirection.x -= -40;
    return true;
  case OIS::KC_S:
    if (vert) {
      mPlayer->mDirection -= btVector3(0, -40, 0);
      mPlayer->oDirection.y -= -40;
    } else {
      mPlayer->mDirection -= btVector3(0, 0, -40);
      mPlayer->oDirection.z -= -40;
    }
    return true;
  case OIS::KC_A:
    mPlayer->mDirection -= btVector3(40, 0, 0);
    mPlayer->oDirection.x -= 40;
    return true;
  case OIS::KC_W:
    if (vert) {
      mPlayer->mDirection -= btVector3(0, 40, 0);
      mPlayer->oDirection.y -= 40;
    } else {
      mPlayer->mDirection -= btVector3(0, 0, 40);
      mPlayer->oDirection.z -= 40;
    }
    return true;
  case OIS::KC_M:
    Sounds::enabled = !Sounds::enabled;
    return true;
  case OIS::KC_LSHIFT:
    mPlayer->movementSpeed = 1;
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

  return false;
}

bool HostApp::mouseMoved( const OIS::MouseEvent& arg ) {
  return handleMouseMoved(arg.state, myId);
}

bool HostApp::handleMouseMoved( OIS::MouseState arg, int userID ) {
  Player *mPlayer = findPlayer(userID);
  if (!mPlayer) return false;

  if (mPlayer->swing == 0 && mPlayer->unswing == 0) {
    int x = arg.X.rel;
    int y = arg.Y.rel;

    static float rotfactor = 6.28 / 1800;

    if (mPlayer->pongMode) {
      //Boundaries
      if(x < 0 && mPlayer->getRacquet()->getPosition().getX() >= 2000){
        x = 0;
      }
      if(x > 0 && mPlayer->getRacquet()->getPosition().getX() <= -2000){
        x = 0;
      }
      if(y < 0 && mPlayer->getRacquet()->getPosition().getY() >= 2000){
        y = 0;
      }
      if(y > 0 && mPlayer->getRacquet()->getPosition().getY() <= -2000){
        y = 0;
      }



      mPlayer->getNode()->translate(btVector3(-x,-y,0));
    } else {
      mPlayer->getNode()->rotate(btQuaternion(btVector3(0,0,1), btScalar(x*rotfactor)));
    }
  }
  return true;
}

bool HostApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  return handleMouseReleased(arg.state, id, myId);
}

bool HostApp::handleMouseReleased( OIS::MouseState arg, OIS::MouseButtonID id, int userID ) {
  Player *mPlayer = findPlayer(userID);
  if (!mPlayer) return false;

  if(mPlayer->swing == 0 && mPlayer->unswing == 0) {
    if(id == OIS::MB_Left || id == OIS::MB_Right) {
      mPlayer->swing = SWING_DELAY;
      Sounds::playSound(Sounds::RACQUET_SWOOSH, 100);

      Ogre::Vector3 p = mPlayer->getRacquet()->getNode()->getPosition();
      mPlayer->axis = new btVector3(p[1], -p[0], 0);

      mPlayer->right_mouse_button = (id == OIS::MB_Right);
    }
  }

  return false;
}

void HostApp::createNewScoringPlane(int points, btVector3 pos, btVector3 speed, btVector3 linearFactor, btVector3 angularFactor) {
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
void HostApp::createScene(void)
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

  myId = 0;
  Player *mPlayer = addPlayer(myId);
  addPlayer(1);

  mBall = new Ball(mSceneMgr, "Ball", "BallNode", 0, mPhysics,
                   btVector3(100,100,150),
                   btVector3( rand() % 120 - 60, rand() % 80 - 40, 6000),
                   1000);

  mBall->getNode()->attachObject(mSceneMgr->createParticleSystem("fountain1", "Examples/PurpleFountain"));
  mBall->getNode()->attachObject(mSceneMgr->createParticleSystem("fountain2", "Examples/PurpleFountain"));

  if (mPlayer->pongMode) mPlayer->getNode()->getEntity()->setVisible(false);

  createNewScoringPlane(2, btVector3( 0, rand() % 3500 - 2000, 5000/2 - 5));
  createNewScoringPlane(4, btVector3( 0, rand() % 3500 - 2000, 5000/2 - 5), btVector3(30,0,0));

}

bool HostApp::frameStarted(const Ogre::FrameEvent &evt) {

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

  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *mPlayer = players[i];
    if (!mPlayer) continue; 

    //store original vectors
    int oldZ = mPlayer->mDirection.getZ();
    int oldX = mPlayer->mDirection.getX();
    int oldY = mPlayer->mDirection.getY();

    //boundaries
    if(mPlayer->getRacquet()->getPosition().getZ() >= 600){
      mPlayer->mDirection.setZ(-10);
    }
    if(mPlayer->getRacquet()->getPosition().getZ() <= -2400){
      mPlayer->mDirection.setZ(10);
    }
    if(mPlayer->getRacquet()->getPosition().getX() >= 2000){
      mPlayer->mDirection.setX(-10);
    }
    if(mPlayer->getRacquet()->getPosition().getX() <= -2000){
      mPlayer->mDirection.setX(10);
    }
    if(mPlayer->getRacquet()->getPosition().getY() >= 2000){
      mPlayer->mDirection.setY(-10);
    }
    if(mPlayer->getRacquet()->getPosition().getY() <= -2000){
      mPlayer->mDirection.setY(10);
    }

    mPlayer->getNode()->getBody()->translate(mPlayer->mDirection*mPlayer->movementSpeed);
    mPlayer->getNode()->translate(mPlayer->mDirection*mPlayer->movementSpeed);

    //reset the vector after translation
    mPlayer->mDirection.setZ(oldZ);
    mPlayer->mDirection.setX(oldX);
    mPlayer->mDirection.setY(oldY);


    // Swings
    if(mPlayer->unswing > 0){
      if (mPlayer->pongMode || mPlayer->right_mouse_button) {
        mPlayer->getNode()->translate(btVector3(0, 0, -25));
      } else if (mPlayer->axis) {
        mPlayer->getNode()->rotate(btQuaternion(*mPlayer->axis, btScalar(-0.1)));
      }
      mPlayer->unswing--;
    }

    if(mPlayer->swing > 0){
      if (mPlayer->pongMode || mPlayer->right_mouse_button) {
        mPlayer->getNode()->translate(btVector3(0, 0, 50));
      } else if (mPlayer->axis) {
        mPlayer->getNode()->rotate(btQuaternion(*mPlayer->axis, btScalar(0.2)));
      }
      mPlayer->swing--;
      if(mPlayer->swing == 0)
        mPlayer->unswing = UNSWING_DELAY;
    }
  }
  mDetailsPanel->setParamValue(DETAILS_LASTSCORE, std::to_string(lastscore));

  mDetailsPanel->setParamValue(DETAILS_SCORE, std::to_string(score));
  if (score > highscore) {
    highscore = score;
    mDetailsPanel->setParamValue(DETAILS_HIGHSCORE, std::to_string(highscore));
  }

  Player *mPlayer = players[myId];
  if (mPlayer) {
    btVector3 pos = (mPlayer->pongMode ? mPlayer->getRacquet()->getPosition() : mPlayer->getNode()->getPosition());
    mCamera->lookAt(pos[0], pos[1], pos[2]);
  }

  if(sd){
    ServerPacket msg;
    btVector3 ballPos = mBall->getPosition();
    msg.type = SERVER_UPDATE;
    msg.ballPos = ballPos;
    
    for (int i = 0; i < MAX_PLAYERS; i++) {
      Player *mPlayer = players[i];
      if (mPlayer) {
        btVector3 playerPos = mPlayer->getNode()->getPosition();
        btQuaternion playerOrientation = mPlayer->getNode()->getOrientation();  
        msg.players[i].nodePos = playerPos;
        msg.players[i].nodeOrientation = playerOrientation;
      }
    }
      
    HostApp::Send((char*)&msg, sizeof(msg));

    /*
      ClientPacket cmsg;
      if(SDLNet_TCP_Recv(sd, &cmsg, sizeof(cmsg)) > 0) {
      std::cout << "Received msg" << std::endl;
      }*/
  }

  return result;
}
