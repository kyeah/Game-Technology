#include "Interpolator.h"
#include "MenuActivity.h"
#include "HostPlayerActivity.h"
#include "OBAnimationManager.h"
#include "Networking.h"
#include "Sounds.h"
#include "common.h"


HostPlayerActivity::HostPlayerActivity(OgreBallApplication *app, const char* levelName) : Activity(app) {
  MAX_TILT = .10;  //Increasing this increases the maximum degree to which the level can rotate
  tiltDelay = 300;  // Increasing this increases the time it takes for the level to rotate

  currentLevelName = std::string(levelName, std::strlen(levelName));
  menuActive = false;
  ceguiActive = false;

  Networking::serverConnect();
  inGame = false;
  chatFocus = false;
}

HostPlayerActivity::~HostPlayerActivity(void) {
  ServerPacket msg;//  msg.type = SERVER_CLOSED;
  for(int i = 1; i < MAX_PLAYERS; i++) {
    if(players[i]) {
      Networking::Send(players[i]->csd, (char*)&msg, sizeof(msg));
    }
  }
  Networking::Close();
  close();
}

void HostPlayerActivity::close(void) {
  //  delete player;
  //  delete mCameraObj;
}

Player* HostPlayerActivity::addPlayer(int userID) {
  Player* mPlayer = new Player(userID);

  mPlayer->character = 0;
  mPlayer->currTiltDelay = tiltDelay;
  mPlayer->lastTilt = btQuaternion(0,0,0);
  mPlayer->currTilt = btQuaternion(0,0,0);
  mPlayer->tiltDest = btQuaternion(0,0,0);

  players[userID] = mPlayer;
  return mPlayer;
}

void HostPlayerActivity::start(void) {
  Sounds::playBackground("media/OgreBall/sounds/StandardLevel.mp3", Sounds::volume);

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
                     CEGUI::Event::Subscriber(&HostPlayerActivity::ExitToMenu, this));

  pauseReturn->removeEvent(CEGUI::PushButton::EventClicked);
  pauseReturn
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&HostPlayerActivity::togglePauseMenu, this));

  lobbySheet = app->Wmgr->getWindow("GameLobby");
  lobbySelectLevel = app->Wmgr->getWindow("GameLobby/SelectLevel");
  lobbySelectCharacter = app->Wmgr->getWindow("GameLobby/SelectCharacter");
  lobbyLeave = app->Wmgr->getWindow("GameLobby/Leave");
  lobbyStart = app->Wmgr->getWindow("GameLobby/Ready");
  lobbyStart->setText("Start");

  lobbyLeave->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&HostPlayerActivity::ExitToMenu, this));

  for (int i = 0; i < 4; i++) {
    std::stringstream ss;
    ss << "GameLobby/" << i+1;
    lobbyPlayerWindows[i] = app->Wmgr->getWindow(ss.str());
  }

  // TODO: Add Chatbox Window to layout
  CEGUI::MouseCursor::getSingleton().show();
  CEGUI::System::getSingleton().setGUISheet(lobbySheet);
  //  loadLevel(currentLevelName);
}

void HostPlayerActivity::handleLobbyState(void) {

  // Handle New Connections
  TCPsocket csd_t = SDLNet_TCP_Accept(Networking::server_socket);
  if(csd_t){

    remoteIP = SDLNet_TCP_GetPeerAddress(csd_t);
    if(remoteIP){
      printf("Successfully connected to %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
    }

    PingMessage ping;

    if(SDLNet_TCP_Recv(csd_t, &ping, sizeof(ping)) > 0) {

      if (!ping.isJoining) {
        PingResponseMessage response;
        strcpy(response.lobbyName, "TEMPORARY SWAGGY P");

        response.numPlayers = 0;
        for (int i = 0; i < MAX_PLAYERS; i++) {
          if (players[i]) response.numPlayers++;
        }

        response.maxPlayers = MAX_PLAYERS;
        Networking::Send(csd_t, (char*)&response, sizeof(response));
        SDLNet_TCP_Close(csd_t);

      } else {

        // Add a new player to the lobby
        if (SDLNet_TCP_AddSocket(Networking::server_socketset, csd_t) == -1) {
          printf("SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError());

        } else {
          for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!players[i]) {
              lobbyPlayerWindows[i]->setText("Player HIII"); // Should receive name in Ping Message

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
              //strcpy(packet.level, currentLevelName.c_str());
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
    }
  }
}

void HostPlayerActivity::loadLevel(const char* name) {
  currentLevelName = std::string(name, std::strlen(name));
  app->destroyAllEntitiesAndNodes();
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

      players[i]->setBall(new OgreBall(app->mSceneMgr, ss.str(), ss.str(), "penguin.mesh",  0, app->mPhysics,
                                       app->levelLoader->playerStartPositions[0], btVector3(1,1,1), btVector3(0,0,0),
                                       16000.0f, 0.5f, btVector3(0,0,0), &app->levelLoader->playerStartRotations[0]));
    }
  }

  app->mCamera->setPosition(Ogre::Vector3(0,0,0));
  mCameraObj = new CameraObject(app->mCameraLookAtNode, app->mCameraNode,
                                (Ogre::Vector3)players[0]->getBall()->getPosition(), app->levelLoader->cameraStartPos);

  gameEnded = false;
  countdown = 2000;
  readyWindow->setAlpha(0.0);
  goWindow->setAlpha(0.0);

  menuActive = false;
  ceguiActive = false;
}

bool HostPlayerActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);
  return true;
}

bool HostPlayerActivity::frameStarted( Ogre::Real elapsedTime ) {
  if (!inGame) {
    handleLobbyState();
    return true;
  }

  if (countdown != -1 && !menuActive && !ceguiActive) {
    int lastcountdown = countdown;
    countdown = std::max((int)(countdown - elapsedTime), -1);
    if (lastcountdown > 1750 && countdown <= 1750) {
      OBAnimationManager::startAnimation("SpinPopin", readyWindow, 0.8);
    } else if (lastcountdown > 0 && countdown <= 0) {
      OBAnimationManager::startAnimation("Popin", goWindow);
    }
  }

  timeLeft = std::max(timeLeft - elapsedTime, 0.0f);
  /*
    if (!gameEnded  && countdown == -1) {
    timeLeft = std::max(timeLeft - elapsedTime, 0.0f);

    if (timeLeft == 0.0f) {
    handleGameOver();
    } else if (app->levelLoader->fallCutoff > player->getPosition()[1]) {
    lives--;
    if (lives < 0) {
    handleGameOver();
    } else {
    OBAnimationManager::startAnimation("SpinPopup", livesDisplay);
    loadLevel(currentLevelName.c_str());
    return true;
    }
    }
    }
  */

  // Update HUD
  std::stringstream sst;
  sst << "SCORE: " << score;
  scoreDisplay->setText(sst.str());

  /*
  std:: stringstream livesSS;
  livesSS << lives << (lives != 1 ? " Lives" : " Life");
  livesDisplay->setText(livesSS.str());
  */

  std::stringstream css;
  css << collectibles << "/" << app->levelLoader->numCollectibles;
  collectDisplay->setText(css.str());

  std::stringstream timess;
  int seconds = std::round(timeLeft/1000);
  int millis = std::min((float)99.0, (float)std::round(fmod(timeLeft,1000)/10));

  timess << seconds << ":";
  if (millis < 10) timess << "0";
  timess << millis;

  timeDisplay->setText(timess.str());

  // Update tilts
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (players[i]) {
      Player *player = players[i];

      player->currTilt = Interpolator::interpQuat(player->currTiltDelay, elapsedTime,
                                                  tiltDelay,
                                                  player->lastTilt,
                                                  player->tiltDest);
    }
  }

  Player *player = players[myId];

  /* TODO: Keep Local ghost camera for every player, or have them update and send their state to us?
  // Set player's gravity based on the direction they are facing
  Ogre::Vector3 ocam = app->mCameraNode->_getDerivedPosition();
  btVector3 facingDirection = player->mOgreBall->getPosition() - btVector3(ocam[0], ocam[1], ocam[2]);
  facingDirection[1] = 0;
  facingDirection.normalize();

  btScalar yaw = btVector3(0,0,-1).angle(facingDirection);
  btQuaternion q((facingDirection[0] > 0 ? -yaw : yaw),0,0);
  q.normalize();

  if (gameEnded) {
  player->getBody()->setGravity(btVector3(0, 1000, 0));
  } else if (countdown == -1) {
  btVector3 tweakedGrav = 2*app->mPhysics->getDynamicsWorld()->getGravity()
  .rotate(currTilt.getAxis(), -3*currTilt.getAngle())
  .rotate(q.getAxis(), q.getAngle());

  tweakedGrav[1] /= 2;
  player->getBody()->setGravity(tweakedGrav);

  }

  // Update Camera Position
  //comment out the lines below if you're building a level; also return false in mouseMoved.
  mCameraObj->update((Ogre::Vector3)player->getPosition(), elapsedTime);

  // This only works in this method, not from CameraObject. DONT ASK JUST ACCEPT
  app->mCameraNode->lookAt((Ogre::Vector3)player->getPosition() + Ogre::Vector3(0,250,0), Ogre::SceneNode::TS_WORLD);
  app->mCamera->lookAt((Ogre::Vector3)player->getPosition() + Ogre::Vector3(0,250,0));

  // Tilt Camera to simulate level tilt
  if (countdown == -1) {
  Ogre::Quaternion oq = Ogre::Quaternion(q.w(), q.x(), q.y(), q.z());
  Ogre::Quaternion noq = Ogre::Quaternion(-q.w(), q.x(), q.y(), q.z());

  Ogre::Real xTilt = currTilt.x();
  if (xTilt < 0) xTilt /= 3;
  Ogre::Quaternion notilt = Ogre::Quaternion(-currTilt.w(),
  xTilt,
  currTilt.y(),
  currTilt.z());

  if (!oq.isNaN() && !notilt.isNaN() && !noq.isNaN()) {
  app->mCameraLookAtNode->rotate(oq);
  app->mCameraLookAtNode->rotate(notilt*notilt);
  app->mCameraLookAtNode->rotate(noq);
  }
  }
  */

  //Player *mPlayer = players[myId];
  //if (mPlayer) {
  // btVector3 pos = (mPlayer->pongMode ? mPlayer->getRacquet()->getPosition() : mPlayer->getNode()->getPosition());
  // mCamera->lookAt(pos[0], pos[1], pos[2]);
  //}

  /* // Don't accept new connections in game
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
  packet.level = currentLevelName;
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
  */

  //RECEIVE INPUTS
  while (SDLNet_CheckSockets(Networking::server_socketset, 1) > 0) {
    for (int i = 1; i < MAX_PLAYERS; i++) {
      if (players[i]) {
        TCPsocket csd = players[i]->csd;
        if (SDLNet_SocketReady(csd)) {
          ClientPacket cmsg;
          ServerPacket closemsg;

          if(SDLNet_TCP_Recv(csd, &cmsg, sizeof(cmsg)) > 0) {
            //TODO: DO STUFF FOR RECEIVED PACKETS
          }
        }
      }
    }
  }

  ServerPacket msg;
  //TODO: NEED TO SETUP MESSAGE THING

  // UPDATE ALL CLIENTS
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *mPlayer = players[i];
    if (mPlayer) {
      //TODO: update clients based off of received thing?
      /* btVector3 playerPos = mPlayer->getNode()->getPosition();
         btQuaternion playerOrientation = mPlayer->getNode()->getOrientation();
         msg.players[i].nodePos = playerPos;
         msg.players[i].nodeOrientation = playerOrientation;
      */}
  }

  for (int i = 1; i < MAX_PLAYERS; i++) {
    if (players[i]) {
      Networking::Send(players[i]->csd, (char*)&msg, sizeof(msg));
    }
  }
  true;
}

//-------------------------------------------------------------------------------------

bool HostPlayerActivity::togglePauseMenu( const CEGUI::EventArgs& e ) {
  togglePauseMenu();
  return true;
}

void HostPlayerActivity::togglePauseMenu( ) {
  menuActive = !menuActive;
  if (menuActive) {
    CEGUI::MouseCursor::getSingleton().show();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("PauseMenu"));
    app->Wmgr->getWindow("PauseMenu/Quit")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&HostPlayerActivity::ExitToMenu, this));
    app->Wmgr->getWindow("PauseMenu/Return")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&HostPlayerActivity::togglePauseMenu, this));
  } else {
    CEGUI::MouseCursor::getSingleton().hide();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("SinglePlayerHUD"));
  }
}

bool HostPlayerActivity::ExitToMenu( const CEGUI::EventArgs& e ) {
  app->switchActivity(new MenuActivity(app));
  return true;
}

void HostPlayerActivity::handleGameEnd() {
  ceguiActive = true;
  gameEnded = true;

  CEGUI::MouseCursor::getSingleton().show();
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("GameWon"));

  app->Wmgr->getWindow("GameWon/BackToMenu")
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&HostPlayerActivity::ExitToMenu, this));

  /*  app->Wmgr->getWindow("GameWon/NextLevel")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&HostPlayerActivity::nextLevel, this));*/
}

//-------------------------------------------------------------------------------------

bool HostPlayerActivity::keyPressed( const OIS::KeyEvent &arg )
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
    return true;
  }

  return handleKeyPressed(arg.key, myId);
}

bool HostPlayerActivity::handleKeyPressed( OIS::KeyCode arg, int userId ) {
  Player *player = players[userId];
  if (!player) return false;

  switch(arg){
  case OIS::KC_D:
    player->tiltDest *= btQuaternion(0,0,-MAX_TILT);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_A:
    player->tiltDest *= btQuaternion(0,0,MAX_TILT);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_W:
    player->tiltDest *= btQuaternion(0,-MAX_TILT,0);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_S:
    player->tiltDest *= btQuaternion(0,MAX_TILT,0);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  default:
    return false;
  }

  return true;
}
//-------------------------------------------------------------------------------------

bool HostPlayerActivity::keyReleased( const OIS::KeyEvent &arg )
{
  if (!inGame || chatFocus || menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectKeyUp(arg.key);
  }

  return handleKeyReleased(arg.key, myId);
}

bool HostPlayerActivity::handleKeyReleased( OIS::KeyCode arg, int userId ) {
  Player *player = players[userId];
  if (!player) return false;

  switch(arg){
  case OIS::KC_D:
    player->tiltDest *= btQuaternion(0,0,MAX_TILT);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_A:
    player->tiltDest *= btQuaternion(0,0,-MAX_TILT);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_W:
    player->tiltDest *= btQuaternion(0,MAX_TILT,0);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  case OIS::KC_S:
    player->tiltDest *= btQuaternion(0,-MAX_TILT,0);
    player->lastTilt = player->currTilt;
    player->currTiltDelay = 0;
    break;
  default:
    return false;
  }

  return true;
}

//-------------------------------------------------------------------------------------

bool HostPlayerActivity::mouseMoved( const OIS::MouseEvent &arg )
{
  if (!inGame || menuActive || ceguiActive) {
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

bool HostPlayerActivity::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (!inGame || menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonDown(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------

bool HostPlayerActivity::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (!inGame || menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonUp(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}
