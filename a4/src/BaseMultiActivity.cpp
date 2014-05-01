#include "Interpolator.h"
#include "MenuActivity.h"
#include "ClientPlayerActivity.h"
#include "OBAnimationManager.h"
#include "common.h"
#include "Networking.h"
#include "Sounds.h"

BaseMultiActivity::BaseMultiActivity(OgreBallApplication *app) : Activity(app) {
  MAX_TILT = .10;  //Increasing this increases the maximum degree to which the level can rotate
  tiltDelay = 300;  // Increasing this increases the time it takes for the level to rotate

  menuActive = false;
  ceguiActive = false;

  lives = 10;
  inGame = false;
  chatFocus = false;

  for (int i = 0; i < MAX_PLAYERS; i++)
    players[i] = 0;

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
                     CEGUI::Event::Subscriber(&BaseMultiActivity::ExitToMenu, this));

  pauseReturn->removeEvent(CEGUI::PushButton::EventClicked);
  pauseReturn
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&BaseMultiActivity::togglePauseMenu, this));

  gameWonSheet = app->Wmgr->getWindow("GameWon");
  gwGoal = app->Wmgr->getWindow("GameWon/Goal");
  gwNextLevel = app->Wmgr->getWindow("GameWon/NextLevel");
  gwBackToMenu = app->Wmgr->getWindow("GameWon/BackToMenu");
  gwViewLeaderboard = app->Wmgr->getWindow("GameWon/ViewLeaderboard");
  gwTimeTaken = app->Wmgr->getWindow("GameWon/TimeTaken");
  gwCollectibles = app->Wmgr->getWindow("GameWon/Collectibles");
  gwBonus = app->Wmgr->getWindow("GameWon/Bonus");
  gwScore = app->Wmgr->getWindow("GameWon/Score");
  gwHighscore = app->Wmgr->getWindow("GameWon/Highscore");
  gwNameEditText = app->Wmgr->getWindow("GameWon/EnterName");
  gwSubmitHighscore = app->Wmgr->getWindow("GameWon/SubmitHighscore");

  gwBackToMenu->removeEvent(CEGUI::PushButton::EventClicked);
  gwBackToMenu
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&BaseMultiActivity::ExitToMenu, this));

  gwNextLevel->removeEvent(CEGUI::PushButton::EventClicked);
  gwNextLevel
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&BaseMultiActivity::SwitchToLobby, this));

  lobbySheet = app->Wmgr->getWindow("GameLobby");
  lobbySelectLevel = app->Wmgr->getWindow("GameLobby/SelectLevel");
  lobbySelectCharacter = app->Wmgr->getWindow("GameLobby/SelectCharacter");
  lobbyLeave = app->Wmgr->getWindow("GameLobby/Leave");
  lobbyStart = app->Wmgr->getWindow("GameLobby/Ready");

  lobbySelectLevel->setVisible(false);

  lobbyLeave->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&BaseMultiActivity::ExitToMenu, this));

  for (int i = 0; i < 4; i++) {
    std::stringstream ss;
    ss << "GameLobby/" << i+1;
    lobbyPlayerWindows[i] = app->Wmgr->getWindow(ss.str());
    lobbyPlayerWindows[i]->setText("");
  }
}

Player* BaseMultiActivity::addPlayer(int userID, const char* name) {
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

BaseMultiActivity::~BaseMultiActivity(void) {
  close();
}

void BaseMultiActivity::close(void) {
  Networking::Close();
}

void BaseMultiActivity::start(void) {
  // TODO: Add Chatbox Window to layout
  CEGUI::MouseCursor::getSingleton().show();
  CEGUI::System::getSingleton().setGUISheet(lobbySheet);
}

bool BaseMultiActivity::SwitchToLobby( const CEGUI::EventArgs& e ) {
  Sounds::playBackground("media/OgreBall/sounds/Menu.mp3", Sounds::volume);
  CEGUI::MouseCursor::getSingleton().show();
  CEGUI::System::getSingleton().setGUISheet(lobbySheet);
}

void BaseMultiActivity::handleLobbyState() {
}

// handleWaiting()

void BaseMultiActivity::loadLevel(const char* name) {
  currentLevelName = std::string(name, std::strlen(name));
  app->destroyAllEntitiesAndNodes();
  app->levelLoader->currObjID = 0;  // VERY IMPORTANT TO ENSURE CONSISTENT OBJECT NAMES ACROSS HOST AND CLIENTS
  app->levelLoader->loadLevel(name);

  levelDisplay->setText(currentLevelName.c_str());

  timeLeft = 60000;  // TODO: Should get timeLeft from level script
  collectibles = 0;
  score = 0;

  gameEnded = false;
  countdown = 2000;
  readyWindow->setAlpha(0.0);
  goWindow->setAlpha(0.0);

  menuActive = false;
  ceguiActive = false;
}

bool BaseMultiActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);
  return true;
}

bool BaseMultiActivity::frameStarted( Ogre::Real elapsedTime ) {
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

bool BaseMultiActivity::togglePauseMenu( const CEGUI::EventArgs& e ) {
  togglePauseMenu();
  return true;
}

void BaseMultiActivity::togglePauseMenu( ) {
  menuActive = !menuActive;
  if (menuActive) {
    CEGUI::MouseCursor::getSingleton().show();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("PauseMenu"));
    app->Wmgr->getWindow("PauseMenu/Quit")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&BaseMultiActivity::ExitToMenu, this));
    app->Wmgr->getWindow("PauseMenu/Return")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&BaseMultiActivity::togglePauseMenu, this));
  } else {
    CEGUI::MouseCursor::getSingleton().hide();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("SinglePlayerHUD"));
  }
}

bool BaseMultiActivity::ExitToMenu( const CEGUI::EventArgs& e ) {
  app->switchActivity(new MenuActivity(app));
  return true;
}

void BaseMultiActivity::handleCrossedFinishLine() {
  ceguiActive = true;

  CEGUI::MouseCursor::getSingleton().show();
  CEGUI::System::getSingleton().setGUISheet(gameWonSheet);

  gwGoal->setAlpha(0.0);
  gwNextLevel->setAlpha(0.0);
  gwBackToMenu->setAlpha(0.0);
  gwViewLeaderboard->setAlpha(0.0);
  gwHighscore->setAlpha(0.0);
  gwNameEditText->setAlpha(0.0);
  gwSubmitHighscore->setAlpha(0.0);

  OBAnimationManager::startAnimation("SpinPopup", gwGoal);
  OBAnimationManager::startAnimation("SpinPopup", gwNextLevel, 0.5);
  OBAnimationManager::startAnimation("SpinPopup", gwBackToMenu, 0.5);
  OBAnimationManager::startAnimation("SpinPopup", gwViewLeaderboard, 0.35);

  std::stringstream timess;
  int seconds = std::round((60000-timeLeft)/1000);
  int millis = std::min((float)99.0, (float)std::round(fmod((60000-timeLeft),1000)/10));

  timess << std::left << setw(20) << "Time Taken: " << std::left << seconds << ":";
  if (millis < 10) timess << "0";
  timess << millis;

  gwTimeTaken->setText(timess.str());
  OBAnimationManager::startAnimation("FadeInFromLeft", gwTimeTaken, 1.0, 1.0);
}

void BaseMultiActivity::handleGameEnd() {
  //  ceguiActive = true;
  gameEnded = true;

  /*
    CEGUI::MouseCursor::getSingleton().show();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("GameWon"));

    app->Wmgr->getWindow("GameWon/BackToMenu")
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&BaseMultiActivity::ExitToMenu, this));
  */
  /*  app->Wmgr->getWindow("GameWon/NextLevel")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&BaseMultiActivity::nextLevel, this));*/
}

//-------------------------------------------------------------------------------------

bool BaseMultiActivity::keyPressed( const OIS::KeyEvent &arg )
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

  return false;
}

//-------------------------------------------------------------------------------------

bool BaseMultiActivity::keyReleased( const OIS::KeyEvent &arg )
{
  if (inGame || chatFocus || menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectKeyUp(arg.key);
  }

  return false;
}

//-------------------------------------------------------------------------------------

bool BaseMultiActivity::mouseMoved( const OIS::MouseEvent &arg )
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

bool BaseMultiActivity::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (!inGame || chatFocus || menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonDown(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------

bool BaseMultiActivity::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (!inGame || chatFocus || menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonUp(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}
