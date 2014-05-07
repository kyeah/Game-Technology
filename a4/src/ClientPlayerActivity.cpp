#include "Interpolator.h"
#include "MenuActivity.h"
#include "ClientPlayerActivity.h"
#include "OBAnimationManager.h"
#include "common.h"
#include "Networking.h"
#include "Sounds.h"
#include "SelectorHelper.h"

#include "../libs/MovableTextOverlay.h"
#include "../libs/RectLayoutManager.h"

ClientPlayerActivity::ClientPlayerActivity(OgreBallApplication *app, ConnectAck *ack) : BaseMultiActivity(app) {
  myId = ack->id;
  currentLevelName = std::string(ack->level, strlen(ack->level));

  for (int i = 0; i < 4; i++) {
    if (ack->ids[i]) {
      addPlayer(i, ack->playerInfo[i].name);
      players[i]->ready = ack->playerInfo[i].ready;
      players[i]->character = ack->playerInfo[i].characterChoice;
    }
  }

  lobbyNamebar->setText(ack->level);
  lobbyStart->setText("Ready");

  lobbyStart->removeEvent(CEGUI::PushButton::EventClicked);
  lobbyStart->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&ClientPlayerActivity::toggleReady, this));

  lobbySelectCharacter->removeEvent(CEGUI::PushButton::EventClicked);
  lobbySelectCharacter->subscribeEvent(CEGUI::PushButton::EventClicked,
                                       CEGUI::Event::Subscriber(&ClientPlayerActivity::SwitchToPlayerSelectMenu, this));

  chatEditbox->removeEvent(CEGUI::Editbox::EventTextAccepted);
  chatEditbox->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
                              CEGUI::Event::Subscriber(&ClientPlayerActivity::handleTextSubmitted,this));
}



bool ClientPlayerActivity::SwitchToPlayerSelectMenu( const CEGUI::EventArgs &e ) {
  SelectorHelper::type_flag = SelectorHelper::TYPE_MULTI_CLIENT;
  SelectorHelper::SwitchToPlayerSelectMenu();
}

void ClientPlayerActivity::handlePlayerSelected(int i) {
  CEGUI::System::getSingleton().setGUISheet(lobbySheet);
  players[myId]->character = i;

  ClientPacket msg;
  msg.type = SERVER_PLAYER_MESH_CHANGE;
  msg.userID = myId;
  msg.characterChange = i;

  Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
}

ClientPlayerActivity::~ClientPlayerActivity(void) {
  closeActivity();
}

void ClientPlayerActivity::closeActivity(void) {
  ClientPacket msg;
  msg.type = CLIENT_CLOSE;
  msg.userID = myId;

  Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
  Networking::Close();

  for(int i = 0; i < MAX_PLAYERS; i++) {
    if(players[i]) {
      if (players[i]->textOverlay)
        players[i]->textOverlay->enable(false);

      players[i] = NULL;
    }
  }
  //  delete player;
  //  delete mCameraObj;
}

void ClientPlayerActivity::start(void) {
  BaseMultiActivity::start();
  loadLevel(currentLevelName.c_str());
}

bool ClientPlayerActivity::handleTextSubmitted( const CEGUI::EventArgs &e ) {
  CEGUI::String cmsg = chatEditbox->getText();

  std::stringstream ss;
  ss << players[myId]->name << ": " << cmsg.c_str();
  const char *msg = ss.str().c_str();

  toggleChat();
  chatEditbox->setText("");

  if (cmsg.length()) {
    addChatMessage(msg);

    ClientPacket packet;
    packet.type = CLIENT_CHAT;
    packet.userID = myId;
    strcpy(packet.msg, msg);
    Networking::Send(Networking::client_socket, (char*)&packet, sizeof(packet));
  }

  allowKeyPress = false;
}

bool ClientPlayerActivity::toggleReady( const CEGUI::EventArgs& e ) {
  ClientPacket msg;
  msg.type = CLIENT_TOGGLEREADY;
  msg.userID = myId;
  Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
}

void ClientPlayerActivity::loadLevel(const char* name) {
  BaseMultiActivity::loadLevel(name);

  MovableTextOverlayAttributes *attrs = new MovableTextOverlayAttributes("Attrs1", app->mCamera ,"BlueHighway" , 16,
                                                                         ColourValue::White,"OgreBall/Transparent");

  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (players[i]) {
      std::stringstream ss;
      ss << "Player" << i;
      std::string playerEnt = ss.str();
      ss << "node";

      char* playerChoice = SelectorHelper::CharacterToString(players[i]->character);

      players[i]->setBall(new OgreBall(app->mSceneMgr, ss.str(), ss.str(), playerChoice,  0,
                                       app->mPhysics,
                                       app->levelLoader->playerStartPositions[0], btVector3(1,1,1),
                                       btVector3(0,0,0), 16000.0f, 0.5f, btVector3(0,0,0),
                                       &app->levelLoader->playerStartRotations[0]));

      MovableTextOverlay* p = new MovableTextOverlay(ss.str(), players[i]->name, players[i]->getBall()->getEntity(), attrs);
      p->enable(false); // make it invisible for now
      p->setUpdateFrequency(1);// set update frequency to 0.01 seconds
      players[i]->textOverlay = p;
    }
  }

  if (inGame) {
    app->mCamera->setPosition(Ogre::Vector3(0,0,0));
    app->mCameraNode->_setDerivedPosition(app->levelLoader->cameraStartPos);
  }

  static int first = true;
  
  if (first) {
    first = false; 
  } else {
    for (int i = 1; i < 4; i++)
      if (players[i]) {
        players[i]->ready = false;
        lobbyPlayerWindows[i]->setProperty("BackgroundColours", "tl:FFDB6837 tr:FFDB6837 bl:FFDB6837 br:FFDB6837");
      }
  }
}

bool ClientPlayerActivity::frameStarted( Ogre::Real elapsedTime ) {
  if (!inGame) {
    handleServerUpdates();
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


  handleServerUpdates();

  std::stringstream timess;
  int seconds = std::round(timeLeft/1000);
  int millis = std::min((float)99.0, (float)std::round(fmod(timeLeft,1000)/10));

  timess << seconds << ":";
  if (millis < 10) timess << "0";
  timess << millis;

  timeDisplay->setText(timess.str());

  RectLayoutManager m(0,0, app->mCamera->getViewport()->getActualWidth(),
                      app->mCamera->getViewport()->getActualHeight());

  m.setDepth(0);

  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (players[i]) {
      MovableTextOverlay *p = players[i]->textOverlay;
      p->update(elapsedTime);
      if (p->isOnScreen()) {
        RectLayoutManager::Rect r(p->getPixelsLeft(),
                                  p->getPixelsTop(),
                                  p->getPixelsRight(),
                                  p->getPixelsBottom());

        RectLayoutManager::RectList::iterator it = m.addData(r);

        if (it != m.getListEnd())
          MovableTextOverlay *p = players[i]->textOverlay;

        p->update(elapsedTime);
        if (p->isOnScreen()) {
          RectLayoutManager::Rect r(p->getPixelsLeft(),
                                    p->getPixelsTop(),
                                    p->getPixelsRight(),
                                    p->getPixelsBottom());

          RectLayoutManager::RectList::iterator it = m.addData(r);
          if (it != m.getListEnd())
            {
              p->setPixelsTop((*it).getTop());
              p->enable(true);
            }
          else
            p->enable(false);
        }
        else
          p->enable(false);
      }
    }
  }

  return true;
}

void ClientPlayerActivity::handleServerUpdates() {
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

        app->mCamera->lookAt((Ogre::Vector3)players[msg.camInfo.viewingPlayer]->getBall()->getPosition()
                             + Ogre::Vector3(0,250,0));
        break;
      }
      case SERVER_GAME_START:
        inGame = true;
        loadLevel(currentLevelName.c_str());
        CEGUI::System::getSingleton().setGUISheet(guiSheet);
        guiSheet->addChildWindow(chatWindow);
        CEGUI::MouseCursor::getSingleton().hide();
        break;
      case SERVER_GAME_END:
        handleGameEnd();
        break;
      case SERVER_CLIENT_CONNECT:
        addPlayer(msg.clientID, "name");
        break;
      case CLIENT_TOGGLEREADY:
        togglePlayerReady(msg.clientID);
        break;
      case SERVER_OBJECT_REMOVAL:
        app->mPhysics->getObjects()[msg.clientID]->removeFromSimulator();
        break;
      case SERVER_PLAY_SOUND:
        Sounds::playSoundEffect(msg.msg);
        break;
      case SERVER_CROSSED_FINISH_LINE:
        handleCrossedFinishLine();
        break;
      case SERVER_CLIENT_MESSAGE:
        addChatMessage(msg.msg);
        break;
      case SERVER_PLAYER_MESH_CHANGE:
        players[msg.clientID]->character = msg.characterChange;
        break;
      case SERVER_LEVEL_CHANGE:
        currentLevelName = std::string(msg.msg, strlen(msg.msg));
        loadLevel(currentLevelName.c_str());
        break;
      case SERVER_CLIENT_CLOSED:
        if (inGame) {
          app->mPhysics->removeObject(players[msg.clientID]->getBall());
          app->mSceneMgr->destroyEntity(players[msg.clientID]->getBall()->getEntity());
          app->mSceneMgr->destroyEntity(players[msg.clientID]->getBall()->getHeadEntity());
        }
        players[msg.clientID] = NULL;
        break;
      case SERVER_CLOSED:
        CEGUI::EventArgs args;
        ExitToMenu(args);
        break;
      }

    }
  }
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

void ClientPlayerActivity::handleCrossedFinishLine() {
  BaseMultiActivity::handleCrossedFinishLine();
  gameEnded = true;
}

//-------------------------------------------------------------------------------------

bool ClientPlayerActivity::keyPressed( const OIS::KeyEvent &arg )
{
  if (!BaseMultiActivity::keyPressed(arg)) {
    switch(arg.key){
    case OIS::KC_RETURN:
      if (allowKeyPress) {
        toggleChat();
        chatEditbox->setText("");
      }
      break;
    case OIS::KC_D:
    case OIS::KC_A:
    case OIS::KC_W:
    case OIS::KC_S: {
      ClientPacket msg;
      msg.type = KEY_PRESSED;
      msg.keyArg = arg.key;
      msg.userID = myId;
      Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
      break;
    }
    case OIS::KC_R:
      break;
    default:
      return false;
    }
  }

  return true;
}

//-------------------------------------------------------------------------------------

bool ClientPlayerActivity::keyReleased( const OIS::KeyEvent &arg )
{
  if (!BaseMultiActivity::keyReleased(arg)) {
    switch(arg.key){
    case OIS::KC_D:
    case OIS::KC_A:
    case OIS::KC_W:
    case OIS::KC_S: {
      ClientPacket msg;
      msg.type = KEY_RELEASED;
      msg.keyArg = arg.key;
      msg.userID = myId;
      Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
      break;
    }
    default:
      return false;
    }
  }

  return true;
}

bool ClientPlayerActivity::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  if (!BaseMultiActivity::mousePressed(arg, id)) {
    if (id == OIS::MB_Left) {
      ClientPacket msg;
      msg.type = MOUSE_PRESSED;
      msg.mouseArg = arg.state;
      msg.mouseID = id;
      msg.userID = myId;
      Networking::Send(Networking::client_socket, (char*)&msg, sizeof(msg));
    }
  }

  return true;
}
