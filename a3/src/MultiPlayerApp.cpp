/*
  -----------------------------------------------------------------------------
  Filename:    MultiPlayerApp.cpp
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
#include "MultiPlayerApp.h"
#include "SDL_net.h"
#include "Networking.h"

Ogre::Light* scene_lights[6];

//-------------------------------------------------------------------------------------
MultiPlayerApp::MultiPlayerApp(bool _isHost) : isHost(_isHost)
{
  connected = false;
  mPhysics = new Physics(btVector3(0, -7000, 0));
  MultiPlayerApp::Connect();
}
//-------------------------------------------------------------------------------------
MultiPlayerApp::~MultiPlayerApp(void)
{
  MultiPlayerApp::Close();
}

//-------------------------------------------------------------------------------------
void MultiPlayerApp::createCamera(void){
  BaseApplication::createCamera();
  mCamera->setPosition(0,0,7000);
  mCamera->lookAt(0,0,500);
}

Player* MultiPlayerApp::findPlayer(int userID) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (players[i] && players[i]->getId() == userID) {
      return players[i];
    }
  }
  return NULL;
}

Player* MultiPlayerApp::addPlayer(int userID) {
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
                            playerInitPos, btVector3(0,0,0), 0));

  mPlayer->setRacquet(new Racquet(mSceneMgr, racquetEnt, ssr.str(), mPlayer->getNode()->getNode(), mPhysics,
                                  racquetInitPos));

  players[userID] = mPlayer;
  return mPlayer;
}

void MultiPlayerApp::createScene(void)
{
  printf("in create scene\n");
  // Set the scene's ambient light
  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
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
  int worldLength, worldWidth, worldHeight;
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
      scene_lights[z] = mSceneMgr->createLight(ss.str());
      scene_lights[z]->setType(Ogre::Light::LT_SPOTLIGHT);
      scene_lights[z]->setDiffuseColour(rand(),rand(),rand());
      scene_lights[z]->setSpecularColour(rand(),rand(),rand());
      scene_lights[z]->setDirection(rand(), rand(), rand());
      scene_lights[z]->setSpotlightOuterAngle(Ogre::Radian(0.1));
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
  scene_lights[0]->setDirection(-0.5,0,1);
  scene_lights[1]->setDirection(0.5,0,1);
  scene_lights[0]->setPosition(0,1800,0);
  scene_lights[1]->setPosition(0,1800,0);

  // Right Wall
  scene_lights[2]->setDirection(-1,0,0);
  scene_lights[3]->setDirection(-1,0,-1);
  scene_lights[2]->setPosition(0,-1800,0);
  scene_lights[3]->setPosition(0,-1800,0);

  // Left Wall
  scene_lights[4]->setDirection(1,0,0);
  scene_lights[5]->setDirection(1,0,-1);
  scene_lights[4]->setPosition(0,-1800,0);
  scene_lights[5]->setPosition(0,-1800,0);

  myId = 1;
  addPlayer(0);
  Player *mPlayer = addPlayer(myId);

  mBall = new Ball(mSceneMgr, "Ball", "BallNode", 0, mPhysics,
                   btVector3(100,100,150),
                   btVector3( rand() % 120 - 60, rand() % 80 - 40, 6000),
                   1000);

  printf("setting ball position initially\n");
  mBall->setPosition(btVector3(0,0,0));

  mBall->getNode()->attachObject(mSceneMgr->createParticleSystem("fountain1", "Examples/PurpleFountain"));
  mBall->getNode()->attachObject(mSceneMgr->createParticleSystem("fountain2", "Examples/PurpleFountain"));
  //if (pongMode) mPlayer->getEntity()->setVisible(false);

}

void MultiPlayerApp::Connect(){
  SDLNet_Init();
  if(SDLNet_ResolveHost(&ip, NULL, 65501) == -1) {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(0);
  }
  sd = SDLNet_TCP_Open(&ip);
  if(!sd){
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(0);
  }

  csd = SDLNet_TCP_Accept(sd);
  while(!csd){
    csd = SDLNet_TCP_Accept(sd);
    printf("trying to accept...\n");
    if(csd){
      remoteIP = SDLNet_TCP_GetPeerAddress(csd);
      if(remoteIP){
        printf("Successfully connected to %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
        connected = true;
      }
    }
  }

}

ServerPacket* MultiPlayerApp::Receive(){
  /*  ServerPacket msg;
      if(SDLNet_TCP_Recv(csd, &msg, sizeof(msg)) > 0){
      return &msg;
      }*/
  return NULL;
}

void MultiPlayerApp::Send(char *msg, int len) {
  if(connected){
    printf("sending, %s\n", msg);
    int result = SDLNet_TCP_Send(sd, (void*)msg, len);
    if(result < len)
      printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
  }
}

void MultiPlayerApp::Close(){
  SDLNet_TCP_Close(csd);
  SDLNet_TCP_Close(sd);
  SDLNet_Quit();
  connected = false;
}

void MultiPlayerApp::createFrameListener(void){
  BaseApplication::createFrameListener();
}

bool MultiPlayerApp::keyReleased(const OIS::KeyEvent &arg){
  static bool vert = false;

  switch(arg.key){
  case OIS::KC_R:
  case OIS::KC_P:
  case OIS::KC_J:
  case OIS::KC_D:
  case OIS::KC_S:
  case OIS::KC_A:
  case OIS::KC_W:
  case OIS::KC_LSHIFT:
  case OIS::KC_G:
    ClientPacket msg;
    msg.type = KEY_RELEASED;
    msg.keyArg = arg.key;
    // msg.userId = ;
    Send((char*)&msg, sizeof(msg));
    return true;
  }

  return BaseApplication::keyPressed(arg);
}

bool MultiPlayerApp::mouseMoved( const OIS::MouseEvent& arg ) {
  ClientPacket msg;
  msg.type = MOUSE_MOVED;
  msg.mouseArg = arg.state;
  // msg.userId = ;
  Send((char*)&msg, sizeof(msg));
  return true;
}

bool MultiPlayerApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  ClientPacket msg;
  msg.type = MOUSE_RELEASED;
  msg.mouseArg = arg.state;
  msg.mouseID = id;
  // msg.userId = ;
  Send((char*)&msg, sizeof(msg));
  return true;
}



bool MultiPlayerApp::keyPressed( const OIS::KeyEvent &arg ) {
  static bool vert = false;

  switch(arg.key){
  case OIS::KC_D:
  case OIS::KC_S:
  case OIS::KC_A:
  case OIS::KC_W:
  case OIS::KC_LSHIFT:
  case OIS::KC_SPACE:
    ClientPacket msg;
    msg.type = KEY_PRESSED;
    msg.keyArg = arg.key;
    // msg.userId = ;
    Send((char*)&msg, sizeof(msg));
    return true;
  }

  return BaseApplication::keyPressed(arg);
}



bool MultiPlayerApp::frameStarted(const Ogre::FrameEvent &evt) {
  ServerPacket msg;
  if(SDLNet_TCP_Recv(csd, &msg, sizeof(msg)) > 0){
    mBall->setPosition(msg.ballPos);

    for (int i = 0; i < MAX_PLAYERS; i++) {
      Player *mPlayer = players[i];
      if (mPlayer) {
        mPlayer->getNode()->setPosition(msg.players[i].nodePos);
        mPlayer->getNode()->setOrientation(msg.players[i].nodeOrientation);
      }
    }
  }
  return true;
}
