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
  Networking::serverConnect();
  connected = true;
  Networking::soundState = Sounds::NO_SOUND;
}

HostApp::~HostApp(void)
{
  ServerPacket msg;
  msg.type = SERVER_CLOSED;
  for(int i = 1; i < MAX_PLAYERS; i++) {
    if (players[i]) {
      Networking::Send(players[i]->csd, (char*)&msg, sizeof(msg));
    }
  }
  Networking::Close();
  CEGUI::OgreRenderer::destroySystem();
}

void HostApp::createCamera(void)
{
  BaseMultiplayerApp::createCamera();
  mCamera->setPosition(0,0,-7000);
  mCamera->lookAt(0,0,500);
}

bool HostApp::keyPressed( const OIS::KeyEvent &arg ) {
  if (chatFocus) {
    if (arg.key == OIS::KC_ESCAPE) {
      toggleChat();
      chatEditBox->setText("");
      return true;
    } else {
      CEGUI::System &sys = CEGUI::System::getSingleton();
      sys.injectKeyDown(arg.key);
      sys.injectChar(arg.text);
      return true;
    }
  }
  
  if (arg.key == OIS::KC_ESCAPE) {
    return BaseApplication::keyPressed(arg);
  }
  return handleKeyPressed(arg.key, myId);
}

bool HostApp::handleKeyPressed( OIS::KeyCode arg, int userId ) {
  static bool vert = false;

  Player *mPlayer = findPlayer(userId);
  if (!mPlayer) return false;

  allowKeyRelease = true;

  switch(arg){
  case OIS::KC_RETURN:
    toggleChat();
    chatEditBox->setText("");
    if (chatFocus) {
      mPlayer->mDirection = btVector3(0, 0, 0);
      mPlayer->oDirection.x = 0;
      mPlayer->oDirection.y = 0;
      mPlayer->oDirection.z = 0;
    }
    return true;
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
      Networking::soundState = Sounds::RACQUET_SWOOSH;
    }
    return true;
  }

  return false;
}
bool HostApp::keyReleased(const OIS::KeyEvent &arg){
  if (chatFocus) {
    return CEGUI::System::getSingleton().injectKeyUp(arg.key);
  }

  if (arg.key == OIS::KC_ESCAPE) {
    return BaseApplication::keyReleased(arg);
  }
  return handleKeyReleased(arg.key, myId);
}

bool HostApp::handleKeyReleased(OIS::KeyCode arg, int userID) {
  static bool vert = false;

  if (!allowKeyRelease) {
    allowKeyRelease = true;
    return false;
  }

  Player *mPlayer = findPlayer(userID);
  if (!mPlayer) return false;
  
  switch(arg){
  case OIS::KC_C:
    chatBox->setVisible(!chatBox->isVisible());
    return true;
  case OIS::KC_R:
    restart();
    return true;
  case OIS::KC_P:
    mPlayer->swing = mPlayer->unswing = 0;
    mPlayer->pongMode = !mPlayer->pongMode;
    mPlayer->getNode()->setOrientation(btQuaternion(0,0,0,1));
    mPlayer->getNode()->setPosition(playerInitialPositions[userID]);
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
  int scalar = (userID % 2 == 0 ? 1 : -1);

  if (mPlayer->swing == 0 && mPlayer->unswing == 0) {
    int x = arg.X.rel;
    int y = arg.Y.rel;

    static float rotfactor = 6.28 / 1800;

    if (mPlayer->pongMode) {
      //Boundaries
      if(x < 0 && mPlayer->getRacquet()->getPosition().getX()*scalar >= 2000){
        x = 0;
      }
      if(x > 0 && mPlayer->getRacquet()->getPosition().getX()*scalar <= -2000){
        x = 0;
      }
      if(y < 0 && mPlayer->getRacquet()->getPosition().getY() >= 2000){
        y = 0;
      }
      if(y > 0 && mPlayer->getRacquet()->getPosition().getY() <= -2000){
        y = 0;
      }
      
      mPlayer->getNode()->translate(btVector3(-x*scalar,-y,0));
    } else {
      mPlayer->getNode()->rotate(btQuaternion(btVector3(0,0,1), btScalar(x*rotfactor*scalar)));
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
      soundState = Sounds::RACQUET_SWOOSH;

      Ogre::Vector3 p = mPlayer->getRacquet()->getNode()->getPosition();
      mPlayer->axis = new btVector3(p[1], -p[0], 0);

      mPlayer->right_mouse_button = (id == OIS::MB_Right);
    }
  }

  return false;
}

bool HostApp::handleTextSubmitted( const CEGUI::EventArgs &e ) {
  CEGUI::String cmsg = chatEditBox->getText();

  std::stringstream ss;
  ss << "Player " << myId << ": " << cmsg.c_str();
  const char *msg = ss.str().c_str();

  toggleChat();
  chatEditBox->setText("");
  addChatMessage(msg);

  ServerPacket packet;
  packet.type = SERVER_CLIENT_MESSAGE;
  packet.clientId = myId;
  strcpy(packet.msg, msg);
  for (int i = 1; i < MAX_PLAYERS; i++) {
    if (players[i]) {
      Networking::Send(players[i]->csd, (char*)&packet, sizeof(packet));
    }
  }

  allowKeyRelease = false;  // Don't allow keyrelease without the keypress
}

void HostApp::createScene(void) {
  BaseMultiplayerApp::createScene();
  chatEditBox->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
                              CEGUI::Event::Subscriber(&HostApp::handleTextSubmitted,this));
}

bool HostApp::frameStarted(const Ogre::FrameEvent &evt)
{
  BaseMultiplayerApp::frameStarted(evt);

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

    btVector3 playerDir = (i % 2 == 0 ? btVector3(1,1,1) : btVector3(-1,1,-1));
    int scalar = (i % 2 == 0 ? 1 : -1);
    btScalar playerScalar = btScalar(scalar);

    //store original vectors
    int oldZ = mPlayer->mDirection.getZ();
    int oldX = mPlayer->mDirection.getX();
    int oldY = mPlayer->mDirection.getY();

    //boundaries
    
    if(mPlayer->getRacquet()->getPosition().getZ()*scalar >= 0){
      mPlayer->mDirection.setZ(-10);
    }
    if(mPlayer->getRacquet()->getPosition().getZ()*scalar <= -2400){
      mPlayer->mDirection.setZ(10);
    }
    if(mPlayer->getRacquet()->getPosition().getX()*scalar >= 2000){
      mPlayer->mDirection.setX(-10);
    }
    if(mPlayer->getRacquet()->getPosition().getX()*scalar <= -2000){
      mPlayer->mDirection.setX(10);
    }
    if(mPlayer->getRacquet()->getPosition().getY() >= 2000){
      mPlayer->mDirection.setY(-10);
    }
    if(mPlayer->getRacquet()->getPosition().getY() <= -2000){
      mPlayer->mDirection.setY(10);
    }

    mPlayer->getNode()->getBody()->translate(mPlayer->mDirection*mPlayer->movementSpeed*playerDir);
    mPlayer->getNode()->translate(mPlayer->mDirection*mPlayer->movementSpeed*playerDir);

    //reset the vector after translation
    mPlayer->mDirection.setZ(oldZ);
    mPlayer->mDirection.setX(oldX);
    mPlayer->mDirection.setY(oldY);


    // Swings
    if(mPlayer->unswing > 0){
      if (mPlayer->pongMode || mPlayer->right_mouse_button) {
        mPlayer->getNode()->translate(btVector3(0, 0, -25)*playerDir);
      } else if (mPlayer->axis) {
        mPlayer->getNode()->rotate(btQuaternion(*mPlayer->axis, btScalar(-0.1)*playerScalar));
      }
      mPlayer->unswing--;
    }

    if(mPlayer->swing > 0){
      if (mPlayer->pongMode || mPlayer->right_mouse_button) {
        mPlayer->getNode()->translate(btVector3(0, 0, 50)*playerDir);
      } else if (mPlayer->axis) {
        mPlayer->getNode()->rotate(btQuaternion(*mPlayer->axis, btScalar(0.2)*playerScalar));
      }
      mPlayer->swing--;
      if(mPlayer->swing == 0)
        mPlayer->unswing = UNSWING_DELAY;
    }
  }

  mDetailsPanel->setParamValue(DETAILS_LASTSCORE, std::to_string(lastscore));

  mDetailsPanel->setParamValue(DETAILS_SCORE, std::to_string(score));
  mDetailsPanel->setParamValue(DETAILS_TEAM1, std::to_string(team1Score));
  mDetailsPanel->setParamValue(DETAILS_TEAM2, std::to_string(team2Score));
  if (score > highscore) {
    highscore = score;
    mDetailsPanel->setParamValue(DETAILS_HIGHSCORE, std::to_string(highscore));
  }

  Player *mPlayer = players[myId];
  if (mPlayer) {
    btVector3 pos = (mPlayer->pongMode ? mPlayer->getRacquet()->getPosition() : mPlayer->getNode()->getPosition());
    mCamera->lookAt(pos[0], pos[1], pos[2]);
  }

 // Handle New Connections
  TCPsocket csd_t = SDLNet_TCP_Accept(Networking::server_socket);
  if(csd_t){
    remoteIP = SDLNet_TCP_GetPeerAddress(csd_t);
    if(remoteIP){
      printf("Successfully connected to %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
    }

    if (SDLNet_TCP_AddSocket(Networking::server_socketset, csd_t) == -1) {
      printf("SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());
    } else {
      for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!players[i]) {

          // Send ack with its new user ID
          ConnectAck ack;
          for (int j = 0; j < MAX_PLAYERS; j++) {
            if (players[j])
              ack.ids[j] = 1;
            else
              ack.ids[j] = 0;
          }

          addPlayer(i);
          players[i]->csd = csd_t;
          ack.id = i;
          Networking::Send(csd_t, (char*)&ack, sizeof(ack));

          // Send notifications to rest of players
          ServerPacket packet;
          packet.type = SERVER_CLIENT_CONNECT;
          packet.clientId = i;
          for (int j = 1; j < MAX_PLAYERS; j++) {
            if (i != j && players[j])
              Networking::Send(players[j]->csd, (char*)&packet, sizeof(packet));
          }
          break;
        }
      }
    }
  }

  // RECEIVE INPUTS
  while (SDLNet_CheckSockets(Networking::server_socketset, 1) > 0) {
    for (int i = 1; i < MAX_PLAYERS; i++) {
      if (players[i]) {
        TCPsocket csd = players[i]->csd;
        if (SDLNet_SocketReady(csd)) {
          ClientPacket cmsg;
          ServerPacket closemsg;

          if(SDLNet_TCP_Recv(csd, &cmsg, sizeof(cmsg)) > 0) {
            switch (cmsg.type) {
            case MOUSE_MOVED:
              handleMouseMoved(cmsg.mouseArg, cmsg.userID);
              break;
            case MOUSE_PRESSED:
              break;
            case MOUSE_RELEASED:
              handleMouseReleased(cmsg.mouseArg, cmsg.mouseID, cmsg.userID);
              break;
            case KEY_PRESSED:
              handleKeyPressed(cmsg.keyArg, cmsg.userID);
              break;
            case KEY_RELEASED:
              handleKeyReleased(cmsg.keyArg, cmsg.userID);
              break;
            case CLIENT_CLOSE:
              mPhysics->removeObject(players[cmsg.userID]->getNode());
              mPhysics->removeObject(players[cmsg.userID]->getRacquet());
              mSceneMgr->destroyEntity(players[cmsg.userID]->getNode()->getEntity());
              mSceneMgr->destroyEntity(players[cmsg.userID]->getRacquet()->getEntity());
              players[cmsg.userID] = NULL;

              closemsg.type = SERVER_CLIENT_CLOSED;
              closemsg.clientId = cmsg.userID;
              for (int i = 1; i < MAX_PLAYERS; i++) {
                if (players[i]) {
                  Networking::Send(players[i]->csd, (char*)&closemsg, sizeof(closemsg));
                }
              }

              SDLNet_TCP_Close(csd);
              break;
            case CLIENT_CLEAR_DIR:
              players[cmsg.userID]->mDirection = btVector3(0,0,0);
              players[cmsg.userID]->oDirection.x = 0;
              players[cmsg.userID]->oDirection.y = 0;
              players[cmsg.userID]->oDirection.z = 0;
              break;
            case CLIENT_CHAT:
              addChatMessage(cmsg.msg);

              ServerPacket packet;
              packet.type = SERVER_CLIENT_MESSAGE;
              packet.clientId = myId;
              memcpy(packet.msg, cmsg.msg, 512);
              for (int j = 1; j < MAX_PLAYERS; j++) {
                if (i != j && players[j])
                  Networking::Send(players[j]->csd, (char*)&packet, sizeof(packet));
              }
              break;
            }
          }
        }
      }
    }
  }

  // UPDATE ALL CLIENTS
  ServerPacket msg;
  btVector3 ballPos = mBall->getPosition();
  msg.type = SERVER_UPDATE;
  msg.ballPos = ballPos;
  msg.playSound = soundState;
  msg.team1 = team1Score;
  msg.team2 = team2Score;

  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *mPlayer = players[i];
    if (mPlayer) {
      btVector3 playerPos = mPlayer->getNode()->getPosition();
      btQuaternion playerOrientation = mPlayer->getNode()->getOrientation();
      msg.players[i].nodePos = playerPos;
      msg.players[i].nodeOrientation = playerOrientation;
    }
  }

  for (int i = 1; i < MAX_PLAYERS; i++) {
    if (players[i]) {
      Networking::Send(players[i]->csd, (char*)&msg, sizeof(msg));
    }
  }
  return result;
}
