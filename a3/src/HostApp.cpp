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
HostApp::HostApp(void) : BaseMultiplayerApp::BaseMultiplayerApp()
{
  myId = 0;
  Connect();
}

void HostApp::createCamera(void) 
{
  BaseMultiplayerApp::createCamera();
  mCamera->setPosition(0,0,-7000);
  mCamera->lookAt(0,0,500);
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

bool HostApp::frameStarted(const Ogre::FrameEvent &evt)
{
  BaseMultiplayerApp::frameStarted(evt);

  if (!players[1]) addPlayer(1);

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

    ClientPacket cmsg;
    // Network is not two-way yet :(
    //      if(SDLNet_TCP_Recv(sd, &cmsg, sizeof(cmsg)) > 0) {
    //  std::cout << "Received msg" << std::endl;
    //}
  }

  return result;
}
