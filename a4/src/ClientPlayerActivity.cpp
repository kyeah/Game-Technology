#include "Interpolator.h"
#include "MenuActivity.h"
#include "ClientPlayerActivity.h"
#include "OBAnimationManager.h"
#include "common.h"
#include "Networking.h"

ClientPlayerActivity::ClientPlayerActivity(OgreBallApplication *app, ConnectAck *ack) : Activity(app) {
  MAX_TILT = .10; //Increasing this increases the maximum degree to which the level can rotate
  currTiltDelay = tiltDelay = 300;  // Increasing this increases the time it takes for the level to rotate

  lastTilt = btQuaternion(0,0,0);
  currTilt = btQuaternion(0,0,0);
  tiltDest = btQuaternion(0,0,0);

  myId = ack->id;
  currentLevelName = std::string(ack->level, strlen(ack->level));
  menuActive = false;
  ceguiActive = false;

  levelLoaded = false;
  waitingScreenLoaded = false;
  readyToLoadLevel = false;

  lives = 10;
  inGame = false;
  chatFocus = false;

  for (int i = 0; i < MAX_PLAYERS; i++)
    players[i] = 0;


  lobbySheet = app->Wmgr->getWindow("GameLobby");
  lobbySelectLevel = app->Wmgr->getWindow("GameLobby/SelectLevel");
  lobbySelectCharacter = app->Wmgr->getWindow("GameLobby/SelectCharacter");
  lobbyLeave = app->Wmgr->getWindow("GameLobby/Leave");
  lobbyStart = app->Wmgr->getWindow("GameLobby/Ready");

  lobbySelectLevel->setVisible(false);

  lobbyLeave->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&ClientPlayerActivity::ExitToMenu, this));


  for (int i = 0; i < 4; i++) {
    std::stringstream ss;
    ss << "GameLobby/" << i+1;
    lobbyPlayerWindows[i] = app->Wmgr->getWindow(ss.str());
  }

  for (int i = 0; i < 4; i++) {
    if (ack->ids[i])
      addPlayer(i, ack->playerInfo[i].name);
  }
}

Player* ClientPlayerActivity::addPlayer(int userID, const char* name) {
  Player* mPlayer = new Player(userID);

  strcpy(mPlayer->name, name);
  mPlayer->character = 0;
  mPlayer->currTiltDelay = tiltDelay;
  mPlayer->lastTilt = btQuaternion(0,0,0);
  mPlayer->currTilt = btQuaternion(0,0,0);
  mPlayer->tiltDest = btQuaternion(0,0,0);

  lobbyPlayerWindows[userID]->setText(mPlayer->name);
  players[userID] = mPlayer;
  return mPlayer;
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

void ClientPlayerActivity::start(void) {
  //  Sounds::playBackground("media/OgreBall/sounds/StandardLevel.mp3", Sounds::volume);

  guiSheet = app->Wmgr->getWindow("SinglePlayerHUD");
  scoreDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Score");
  livesDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Lives");
  collectDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Collectibles");
  timeDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Timer");
  levelDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Level");

  readyWindow = app->Wmgr->getWindow("SPHUD/Ready");
  goWindow = app->Wmgr->getWindow("SPHUD/Go");

  pauseMenuSheet = app->Wmgr->getWindow("PauseMenu");
  pauseQuit = app->Wmgr->getWindow("PauseMenu/Quit");
  pauseReturn = app->Wmgr->getWindow("PauseMenu/Return");

  pauseQuit->removeEvent(CEGUI::PushButton::EventClicked);
  pauseQuit
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&ClientPlayerActivity::ExitToMenu, this));

  pauseReturn->removeEvent(CEGUI::PushButton::EventClicked);
  pauseReturn
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&ClientPlayerActivity::togglePauseMenu, this));

  // TODO: Add Chatbox Window to layout
  CEGUI::MouseCursor::getSingleton().show();
  CEGUI::System::getSingleton().setGUISheet(lobbySheet);

  for (int i = 0; i < 4; i++) {
    // Need to replace client_ids with PlayerInfo so we can show name, player mesh choice information
  }

  // Need to keep track of server information to show level selection, etc.

  //  loadLevel(currentLevelName);
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
  currentLevelName = std::string(name, std::strlen(name));
  app->destroyAllEntitiesAndNodes();
  app->levelLoader->currObjID = 0;  // VERY IMPORTANT TO ENSURE CONSISTENT OBJECT NAMES ACROSS HOST AND CLIENTS
  app->levelLoader->loadLevel(name);

  levelDisplay->setText(currentLevelName.c_str());

  timeLeft = 60000;  // TODO: Should get timeLeft from level script
  collectibles = 0;
  score = 0;

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
  /*  mCameraObj = new CameraObject(app->mCameraLookAtNode, app->mCameraNode,
      (Ogre::Vector3)players[0]->getBall()->getPosition(),
      app->levelLoader->cameraStartPos);
  */

  gameEnded = false;
  countdown = 2000;
  readyWindow->setAlpha(0.0);
  goWindow->setAlpha(0.0);

  menuActive = false;
  ceguiActive = false;
}

bool ClientPlayerActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);
  return true;
}

bool ClientPlayerActivity::frameStarted( Ogre::Real elapsedTime ) {
  if (!inGame) {
    handleLobbyState();
    return true;
  } //else if (waitingForLoad) {
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
    if(SDLNet_TCP_Recv(Networking::client_socket, &msg, sizeof(msg)) > 0){
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
        /*        currentLevelName = msg.level;
                  loadLevel(currentLevelName);
                  readyToLoadLevel = true;*/
        addPlayer(msg.clientID, "name");
        break;
      }
      //TODO receive server stuff
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

bool ClientPlayerActivity::togglePauseMenu( const CEGUI::EventArgs& e ) {
  togglePauseMenu();
  return true;
}

void ClientPlayerActivity::togglePauseMenu( ) {
  menuActive = !menuActive;
  if (menuActive) {
    CEGUI::MouseCursor::getSingleton().show();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("PauseMenu"));
    app->Wmgr->getWindow("PauseMenu/Quit")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&ClientPlayerActivity::ExitToMenu, this));
    app->Wmgr->getWindow("PauseMenu/Return")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&ClientPlayerActivity::togglePauseMenu, this));
  } else {
    CEGUI::MouseCursor::getSingleton().hide();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("SinglePlayerHUD"));
  }
}

bool ClientPlayerActivity::waitForHosts(){
  if(!waitingScreenLoaded){
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
  return false;
}

bool ClientPlayerActivity::ExitToMenu( const CEGUI::EventArgs& e ) {
  app->switchActivity(new MenuActivity(app));
  return true;
}

void ClientPlayerActivity::handleGameEnd() {
  ceguiActive = true;
  gameEnded = true;

  CEGUI::MouseCursor::getSingleton().show();
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("GameWon"));

  app->Wmgr->getWindow("GameWon/BackToMenu")
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&ClientPlayerActivity::ExitToMenu, this));

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

//-------------------------------------------------------------------------------------

bool ClientPlayerActivity::mouseMoved( const OIS::MouseEvent &arg )
{
  if (!inGame || chatFocus || menuActive || ceguiActive) {
    CEGUI::System &sys = CEGUI::System::getSingleton();
    sys.injectMouseMove(arg.state.X.rel, arg.state.Y.rel);
    // Scroll wheel.
    if (arg.state.Z.rel)
      sys.injectMouseWheelChange(arg.state.Z.rel / 120.0f);
    return true;
  }

  return true;
}

//-------------------------------------------------------------------------------------

bool ClientPlayerActivity::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (!inGame || chatFocus || menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonDown(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------

bool ClientPlayerActivity::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (!inGame || chatFocus || menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonUp(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}
