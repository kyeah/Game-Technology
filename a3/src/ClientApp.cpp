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
  //  myId = 1;
  Connect();
}

ClientApp::~ClientApp(void)
{
  Close();
}

//-------------------------------------------------------------------------------------
void ClientApp::createCamera(void) {
  BaseMultiplayerApp::createCamera();
  mCamera->setPosition(0,0,7000);
  mCamera->lookAt(0,0,500);
}

void ClientApp::Connect(){
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

  socketset = SDLNet_AllocSocketSet(1);
  if (SDLNet_TCP_AddSocket(socketset, sd) == -1) {
    printf("SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError()); // Probably need to make the socketset bigger
  }

  connected = true;

  ConnectAck ack;
  SDLNet_TCP_Recv(sd, &ack, sizeof(ack));
  myId = ack.id;
  for (int i = 0; i < MAX_PLAYERS; i++) {
    ids[i] = ack.ids[i];
  }
  printf("myID: %d\n", myId);
}

void ClientApp::Close(){
  if (sd) {
    ClientPacket msg;
    msg.type = CLIENT_CLOSE;
    msg.userID = myId;
    Send(sd, (char*)&msg, sizeof(msg));
    SDLNet_TCP_Close(sd);
  }

  SDLNet_Quit();
  connected = false;
  CEGUI::OgreRenderer::destroySystem();
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
    Send(sd, (char*)&msg, sizeof(msg));
    return true;
  }

  return BaseApplication::keyPressed(arg);
}

bool ClientApp::mouseMoved( const OIS::MouseEvent& arg ) {
  ClientPacket msg;
  msg.type = MOUSE_MOVED;
  msg.mouseArg = arg.state;
  msg.userID = myId;
  Send(sd, (char*)&msg, sizeof(msg));
  return true;
}

bool ClientApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  ClientPacket msg;
  msg.type = MOUSE_RELEASED;
  msg.mouseArg = arg.state;
  msg.mouseID = id;
  msg.userID = myId;
  Send(sd, (char*)&msg, sizeof(msg));
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
    Send(sd, (char*)&msg, sizeof(msg));
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
    Send(sd, (char*)&msg, sizeof(msg));
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
  Send(sd, (char*)&packet, sizeof(packet));
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
      if (ids[i] && !players[i]) {
        addPlayer(i);
      }
    }
    first = false;
  }

  while (SDLNet_CheckSockets(socketset, 1) > 0 && SDLNet_SocketReady(sd)) {
    ServerPacket msg;
    if(SDLNet_TCP_Recv(sd, &msg, sizeof(msg)) > 0){
      switch (msg.type) {
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
