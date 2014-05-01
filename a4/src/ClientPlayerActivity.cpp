#include "Interpolator.h"
#include "MenuActivity.h"
#include "ClientPlayerActivity.h"
#include "OBAnimationManager.h"
#include "common.h"
#include "Networking.h"

ClientPlayerActivity::ClientPlayerActivity(OgreBallApplication *app, ConnectAck *ack) : BaseMultiActivity(app) {
  myId = ack->id;
  currentLevelName = std::string(ack->level, strlen(ack->level));

  for (int i = 0; i < 4; i++) {
    if (ack->ids[i])
      addPlayer(i, ack->playerInfo[i].name);
  }

  lobbyStart->setText("Ready");
  lobbyStart->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&ClientPlayerActivity::toggleReady, this));
}

ClientPlayerActivity::~ClientPlayerActivity(void) {
  close();
}

void ClientPlayerActivity::close(void) {
  ClientPacket msg;
  msg.type = CLIENT_CLOSE;
  msg.userID = myId;

  Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
  Networking::Close();

  //  delete player;
  //  delete mCameraObj;
}

bool ClientPlayerActivity::toggleReady( const CEGUI::EventArgs& e ) {
  ClientPacket msg;
  msg.type = CLIENT_TOGGLEREADY;
  msg.userID = myId;
  Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
}

void ClientPlayerActivity::handleLobbyState() {
  while(SDLNet_CheckSockets(Networking::client_socketset, 1) > 0 && SDLNet_SocketReady(Networking::client_socket)){
    ServerPacket msg;
    if(SDLNet_TCP_Recv(Networking::client_socket, &msg, sizeof(msg)) > 0){
      switch(msg.type){
      case SERVER_GAME_START:
        inGame = true;
        loadLevel(currentLevelName.c_str());
        CEGUI::System::getSingleton().setGUISheet(guiSheet);
        CEGUI::MouseCursor::getSingleton().hide();
        break;
      }
    }
  }
}
// handleWaiting()

void ClientPlayerActivity::loadLevel(const char* name) {
  BaseMultiActivity::loadLevel(name);

  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (players[i]) {
      std::stringstream ss;
      ss << "Player" << i;
      std::string playerEnt = ss.str();
      ss << "node";

      players[i]->setBall(new OgreBall(app->mSceneMgr, ss.str(), ss.str(), "penguin.mesh",  0,
                                       app->mPhysics,
                                       app->levelLoader->playerStartPositions[0], btVector3(1,1,1),
                                       btVector3(0,0,0), 16000.0f, 0.5f, btVector3(0,0,0),
                                       &app->levelLoader->playerStartRotations[0]));
    }
  }

  app->mCamera->setPosition(Ogre::Vector3(0,0,0));
  app->mCameraNode->_setDerivedPosition(app->levelLoader->cameraStartPos);
}

bool ClientPlayerActivity::frameStarted( Ogre::Real elapsedTime ) {
  if (!inGame) {
    handleLobbyState();
    return true;
  }

  if (timeLeft <= 0.0f) {
    //handle game over similar to crossed finish line
  }
  //else if (waitingForLoad) {
  //stuff
  //}

  /*
    while(SDLNet_CheckSockets(Networking::client_socketset, 1) > 0 && SDLNet_SocketReady(Networking::client_socket)){
    ServerPacket msg;
    if(SDLNet_TCP_Recv(Networking::client_socket, &msg, sizeof(msg)) > 0){
    switch(msg.type){
    case SERVER_CLIENT_CONNECT:
    currentLevelName = msg.level;
    loadLevel(currentLevelName);
    readyToLoadLevel = true;
    default:
    break;
    }
    //TODO receive server stuff
    }
    }
  */

  /*
    if(!levelLoaded){
    if(!waitForHosts())
    return true;
    }
  */

  if (countdown != -1 && !menuActive && !ceguiActive) {
    int lastcountdown = countdown;
    countdown = std::max((int)(countdown - elapsedTime), -1);
    if (lastcountdown > 1750 && countdown <= 1750) {
      OBAnimationManager::startAnimation("SpinPopin", readyWindow, 0.8);
    } else if (lastcountdown > 0 && countdown <= 0) {
      OBAnimationManager::startAnimation("Popin", goWindow);
    }
  }

  // Update HUD
  /*  std::stringstream sst;
      sst << "SCORE: " << score;
      scoreDisplay->setText(sst.str());
  */
  /*
    std:: stringstream livesSS;
    livesSS << lives << (lives != 1 ? " Lives" : " Life");
    livesDisplay->setText(livesSS.str());
  */
  /*
    std::stringstream css;
    css << collectibles << "/" << app->levelLoader->numCollectibles;
    collectDisplay->setText(css.str());
  */

  while(SDLNet_CheckSockets(Networking::client_socketset, 1) > 0 && SDLNet_SocketReady(Networking::client_socket)){
    ServerPacket msg;
    if(SDLNet_TCP_Recv(Networking::client_socket, &msg, sizeof(msg)) > 0) {
      switch(msg.type){
      case SERVER_UPDATE: {
        timeLeft = msg.timeLeft;
        app->mCameraNode->setPosition(msg.camInfo.position);
        app->mCameraNode->setOrientation(msg.camInfo.orientation);

        std::deque<GameObject*> objects = app->mPhysics->getObjects();
        for (int i = 0; i < objects.size(); i++) {
          objects[i]->setPosition(msg.objectInfo[i].position);
          objects[i]->setOrientation(msg.objectInfo[i].orientation);
        }

        app->mCamera->lookAt((Ogre::Vector3)players[myId]->getBall()->getPosition() + Ogre::Vector3(0,250,0));
        break;
      }
      case SERVER_CLIENT_CONNECT:
        addPlayer(msg.clientID, "name");
        break;
      case CLIENT_TOGGLEREADY:
        players[msg.clientID]->ready = !players[msg.clientID]->ready;
        break;
      case SERVER_CROSSED_FINISH_LINE:
        handleCrossedFinishLine();
        break;
      case SERVER_CLIENT_MESSAGE:
        break;
      case SERVER_PLAYER_MESH_CHANGE:
        //player[msg.clientID]->character = msg.jifjeisf;
        break;
      case SERVER_LEVEL_CHANGE:
        // currentLevelName = std::string(msg.level, strlen(msg.level));
        break;
      case SERVER_CLIENT_CLOSED:
        app->mPhysics->removeObject(players[msg.clientID]->getBall());
        app->mSceneMgr->destroyEntity(players[msg.clientID]->getBall()->getEntity());
        app->mSceneMgr->destroyEntity(players[msg.clientID]->getBall()->getHeadEntity());
        players[msg.clientID] = NULL;
      }
    }
  }


  std::stringstream timess;
  int seconds = std::round(timeLeft/1000);
  int millis = std::min((float)99.0, (float)std::round(fmod(timeLeft,1000)/10));

  timess << seconds << ":";
  if (millis < 10) timess << "0";
  timess << millis;

  timeDisplay->setText(timess.str());

  return true;
}

//-------------------------------------------------------------------------------------

bool ClientPlayerActivity::waitForHosts(){
  /*  if(!waitingScreenLoaded){
      waitingScreenLoaded = true;
      CEGUI::MouseCursor::getSingleton().show();
      CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("WaitingMenu"));
      app->Wmgr->getWindow("WaitingMenu/Quit")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&ClientPlayerActivity::ExitToMenu, this));
      }
      if(readyToLoadLevel){
      levelLoaded = true;
      CEGUI::MouseCursor::getSingleton().hide();
      CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("SinglePlayerHUD"));
      return true;
      }
      return false;*/
}

void ClientPlayerActivity::handleGameEnd() {
  //  ceguiActive = true;
  gameEnded = true;

  /*
    CEGUI::MouseCursor::getSingleton().show();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("GameWon"));

    app->Wmgr->getWindow("GameWon/BackToMenu")
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ClientPlayerActivity::ExitToMenu, this));
  */
  /*  app->Wmgr->getWindow("GameWon/NextLevel")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&ClientPlayerActivity::nextLevel, this));*/
}

//-------------------------------------------------------------------------------------

bool ClientPlayerActivity::keyPressed( const OIS::KeyEvent &arg )
{
  if (arg.key == OIS::KC_ESCAPE) {
    if (chatFocus) {
      // toggleChat();
      // chatEditBox->setText("");
      // return true;
    } else if (inGame) {
      togglePauseMenu();
      return true;
    }
  }

  if (!inGame || chatFocus || ceguiActive || menuActive) {
    CEGUI::System::getSingleton().injectKeyDown(arg.key);
    CEGUI::System::getSingleton().injectChar(arg.text);
  }

  switch(arg.key){
  case OIS::KC_D:
  case OIS::KC_A:
  case OIS::KC_W:
  case OIS::KC_S:
    ClientPacket msg;
    msg.type = KEY_PRESSED;
    msg.keyArg = arg.key;
    msg.userID = myId;
    Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------

bool ClientPlayerActivity::keyReleased( const OIS::KeyEvent &arg )
{
  if (inGame || chatFocus || menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectKeyUp(arg.key);
  }

  switch(arg.key){
  case OIS::KC_D:
  case OIS::KC_A:
  case OIS::KC_W:
  case OIS::KC_S:
    ClientPacket msg;
    msg.type = KEY_RELEASED;
    msg.keyArg = arg.key;
    msg.userID = myId;
    Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
    return true;
  }

  return false;
}
