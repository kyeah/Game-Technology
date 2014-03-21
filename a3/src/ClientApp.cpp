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

//-------------------------------------------------------------------------------------
ClientApp::ClientApp(void) : BaseMultiplayerApp::BaseMultiplayerApp()
{
  myId = 1;
  Connect();
}

//-------------------------------------------------------------------------------------
void ClientApp::createCamera(void) {
  BaseMultiplayerApp::createCamera();
  mCamera->setPosition(0,0,7000);
  mCamera->lookAt(0,0,500);
}

void ClientApp::Connect(){
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

ServerPacket* ClientApp::Receive(){
  /*  ServerPacket msg;
      if(SDLNet_TCP_Recv(csd, &msg, sizeof(msg)) > 0){
      return &msg;
      }*/
  return NULL;
}

void ClientApp::Send(char *msg, int len) {
  if(connected){
    printf("sending, %s\n", msg);
    int result = SDLNet_TCP_Send(sd, (void*)msg, len);
    if(result < len)
      printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
  }
}

void ClientApp::Close(){
  SDLNet_TCP_Close(csd);
  SDLNet_TCP_Close(sd);
  SDLNet_Quit();
  connected = false;
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
    Send((char*)&msg, sizeof(msg));
    return true;
  }

  return BaseApplication::keyPressed(arg);
}

bool ClientApp::mouseMoved( const OIS::MouseEvent& arg ) {
  ClientPacket msg;
  msg.type = MOUSE_MOVED;
  msg.mouseArg = arg.state;
  msg.userID = myId;
  Send((char*)&msg, sizeof(msg));
  return true;
}

bool ClientApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  ClientPacket msg;
  msg.type = MOUSE_RELEASED;
  msg.mouseArg = arg.state;
  msg.mouseID = id;
  msg.userID = myId;
  Send((char*)&msg, sizeof(msg));
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
    Send((char*)&msg, sizeof(msg));
    return true;
  }

  return BaseApplication::keyPressed(arg);
}

bool ClientApp::frameStarted(const Ogre::FrameEvent &evt) {
  BaseMultiplayerApp::frameStarted(evt);

  if (!players[0]) {
    addPlayer(0);
  }

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
