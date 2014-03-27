/*
  -----------------------------------------------------------------------------
  Filename:    ClientApp.cpp
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
#include "ClientApp.h"
#include "SDL_net.h"
#include "Networking.h"
#include "Sounds.h"

//-------------------------------------------------------------------------------------
ClientApp::ClientApp(void) : BaseMultiplayerApp::BaseMultiplayerApp()
{
  //  myId = 1;
  Networking::clientConnect();
  connected = true;
  Sounds::init();
}

//-------------------------------------------------------------------------------------
void ClientApp::createCamera(void) {
  BaseMultiplayerApp::createCamera();
  mCamera->setPosition(0,0,7000);
  mCamera->lookAt(0,0,500);
}

bool ClientApp::keyReleased(const OIS::KeyEvent &arg){
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
    msg.userID = myId;
    Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
    return true;
  }

  return BaseApplication::keyPressed(arg);
}

bool ClientApp::mouseMoved( const OIS::MouseEvent& arg ) {
  ClientPacket msg;
  msg.type = MOUSE_MOVED;
  msg.mouseArg = arg.state;
  msg.userID = myId;
  Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
  return true;
}

bool ClientApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  ClientPacket msg;
  msg.type = MOUSE_RELEASED;
  msg.mouseArg = arg.state;
  msg.mouseID = id;
  msg.userID = myId;
  Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
  return true;
}

bool ClientApp::keyPressed( const OIS::KeyEvent &arg ) {
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
    msg.userID = myId;
    Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
    return true;
  }

  return BaseApplication::keyPressed(arg);
}

bool ClientApp::frameStarted(const Ogre::FrameEvent &evt) {
  BaseMultiplayerApp::frameStarted(evt);

  static bool first = true;
  if (first) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
      if (Networking::client_ids[i] && !players[i]) {
        addPlayer(i);
      }
    }
    first = false;
  }

  //int active = SDLNet_CheckSockets(socketset, 1);
  while (SDLNet_CheckSockets(Networking::client_socketset, 1) > 0 && SDLNet_SocketReady(Networking::client_socket)) {
    ServerPacket msg;
    if(SDLNet_TCP_Recv(Networking::client_socket, &msg, sizeof(msg)) > 0){
      mBall->setPosition(msg.ballPos);

      for (int i = 0; i < MAX_PLAYERS; i++) {
        Player *mPlayer = players[i];
        if (mPlayer) {
          mPlayer->getNode()->setPosition(msg.players[i].nodePos);
          mPlayer->getNode()->setOrientation(msg.players[i].nodeOrientation);
        }
      }
    }
  }

  return true;
}
