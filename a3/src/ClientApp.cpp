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
  myId = 1;
  Networking::clientConnect(&myId);
  connected = true;
  Sounds::init();
}

ClientApp::~ClientApp(void)
{
  Networking::Close();
}

//-------------------------------------------------------------------------------------
void ClientApp::createCamera(void) {
  BaseMultiplayerApp::createCamera();
  mCamera->setPosition(0,0,7000);
  mCamera->lookAt(0,0,500);
}

bool ClientApp::keyReleased(const OIS::KeyEvent &arg){
  static bool vert = false;

  if (chatFocus) {
    if (arg.key == OIS::KC_ESCAPE) {
      toggleChat();
      chatEditBox->setText("");
      return true;
    } else {
      return CEGUI::System::getSingleton().injectKeyUp(arg.key);
    }
  } else if (!allowKeyRelease) {
    allowKeyRelease = true;
    return BaseApplication::keyPressed(arg);
  }

  switch(arg.key){
  case OIS::KC_C:
    chatBox->setVisible(!chatBox->isVisible());
    return true;
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

  if (chatFocus) {
    CEGUI::System &sys = CEGUI::System::getSingleton();
    sys.injectKeyDown(arg.key);
    sys.injectChar(arg.text);
    return true;
  }

  allowKeyRelease = true;

  ClientPacket msg;
  switch(arg.key){
  case OIS::KC_RETURN:
    toggleChat();
    chatEditBox->setText("");
    msg.type = CLIENT_CLEAR_DIR;
    msg.userID = myId;
    Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
    return true;
  case OIS::KC_D:
  case OIS::KC_S:
  case OIS::KC_A:
  case OIS::KC_W:
  case OIS::KC_LSHIFT:
  case OIS::KC_SPACE:
    msg.type = KEY_PRESSED;
    msg.keyArg = arg.key;
    msg.userID = myId;
    Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
    return true;
  }

  return BaseApplication::keyPressed(arg);
}

bool ClientApp::handleTextSubmitted( const CEGUI::EventArgs &e ) {
  CEGUI::String cmsg = chatEditBox->getText();

  std::stringstream ss;
  ss << "Player " << myId << ": " << cmsg.c_str();
  const char *msg = ss.str().c_str();

  toggleChat();
  chatEditBox->setText("");
  addChatMessage(msg);

  ClientPacket packet;
  packet.type = CLIENT_CHAT;
  packet.userID = myId;
  strcpy(packet.msg, msg);
  Networking::Send(Networking::client_socket, (char*)&packet, sizeof(packet));
}

void ClientApp::createScene(void) {
  BaseMultiplayerApp::createScene();
  chatEditBox->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
                              CEGUI::Event::Subscriber(&ClientApp::handleTextSubmitted,this));
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
      switch(msg.type){
	case SERVER_UPDATE: 
        mBall->setPosition(msg.ballPos);
	
        for (int i = 0; i < MAX_PLAYERS; i++) {
          Player *mPlayer = players[i];
          if (mPlayer) {
            mPlayer->getNode()->setPosition(msg.players[i].nodePos);
            mPlayer->getNode()->setOrientation(msg.players[i].nodeOrientation);
          }
        }
      
        break;
      case SERVER_CLIENT_CONNECT:
        addPlayer(msg.clientId);
        break;
      case SERVER_CLIENT_CLOSED:
        mPhysics->removeObject(players[msg.clientId]->getNode());
        mPhysics->removeObject(players[msg.clientId]->getRacquet());
        mSceneMgr->destroyEntity(players[msg.clientId]->getNode()->getEntity());
        mSceneMgr->destroyEntity(players[msg.clientId]->getRacquet()->getEntity());
        players[msg.clientId] = NULL;
        break;
      case SERVER_CLIENT_MESSAGE:
        addChatMessage(msg.msg);
        break;
      case SERVER_CLOSED:
        SDLNet_TCP_Close(sd);
        sd = 0;
        mShutDown = true;
        break;
      }
    }
  }

  return true;
}
