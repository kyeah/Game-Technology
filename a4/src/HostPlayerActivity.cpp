#include "Interpolator.h"
#include "MenuActivity.h"
#include "HostPlayerActivity.h"
#include "Networking.h"
#include "common.h"


HostPlayerActivity::HostPlayerActivity(OgreBallApplication *app, const char* levelName) : Activity(app) {
  MAX_TILT = .10; //Increasing this increases the maximum degree to which the level can rotate
  currTiltDelay = tiltDelay = 300;  // Increasing this increases the time it takes for the level to rotate
  lastTilt = btQuaternion(0,0,0);
  currTilt = btQuaternion(0,0,0);
  tiltDest = btQuaternion(0,0,0);
  currentLevelName = levelName;
  menuActive = false;
  ceguiActive = false;
  bool inGame = false;
  lives = 10;
  Networking::serverConnect();
  inGame = false;
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
  delete player;
  delete mCameraObj;

}

void HostPlayerActivity::start(void) {
  //CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("SinglePlayerHUD"));
  scoreDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Score");
  livesDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Lives");
  collectDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Collectibles");
  timeDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Timer");
  levelDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Level");

  //  loadLevel(currentLevelName);
}

void HostPlayerActivity::loadLevel(const char* name) {

  app->destroyAllEntitiesAndNodes();
  app->levelLoader->loadLevel(name);
  app->mSceneMgr->setSkyDome(true,"Examples/CloudySky", 5, 8);

  levelDisplay->setText(name);

  timeLeft = 60000;  // TODO: Should get timeLeft from level script
  collectibles = 0;  // TODO: Get total number of collectibles when loading level
  score = 0;

  player = new OgreBall(app->mSceneMgr, "player1", "player1", "penguin.mesh", 0, app->mPhysics,
                        app->levelLoader->playerStartPositions[0], btVector3(1,1,1), btVector3(0,0,0),
                        16000.0f, 0.5f, btVector3(0,0,0), &app->levelLoader->playerStartRotations[0]);

  app->mCamera->setPosition(Ogre::Vector3(0,0,0));
  mCameraObj = new CameraObject(app->mCameraLookAtNode, app->mCameraNode,
                                (Ogre::Vector3)player->getPosition(), app->levelLoader->cameraStartPos);

  gameEnded = false;
}

Player* HostPlayerActivity::addPlayer(int userID) {
  Player* mPlayer = new Player(userID);

  std::stringstream ss;
  ss << "Player" << userID;
  std::string playerEnt = ss.str();
  ss << "node";

  mPlayer->setBall(new OgreBall(app->mSceneMgr, ss.str(), ss.str(), "penguin.mesh",  0, app->mPhysics,
			app->levelLoader->playerStartPositions[0], btVector3(1,1,1), btVector3(0,0,0), 
			16000.0f, 0.5f, btVector3(0,0,0), &app->levelLoader->playerStartRotations[0]));

  players[userID] = mPlayer;
  return mPlayer;
}


bool HostPlayerActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  return true;
}

bool HostPlayerActivity::frameStarted( Ogre::Real elapsedTime ) {
  timeLeft = std::max(timeLeft - elapsedTime, 0.0f);
  currTilt = Interpolator::interpQuat(currTiltDelay, elapsedTime, tiltDelay,
                                      lastTilt, tiltDest);
  std::cout << "HEY THERE" << std::endl;
  // Update HUD
  std::stringstream sst;
  sst << "SCORE: " << score;
  scoreDisplay->setText(sst.str());

  std:: stringstream livesSS;
  livesSS << lives << " Lives";
  livesDisplay->setText(livesSS.str());
  collectDisplay->setText(std::to_string(collectibles));

  std::stringstream timess;
  int seconds = std::round(timeLeft/1000);
  int millis = std::min((float)99.0, (float)std::round(fmod(timeLeft,1000)/10));

  timess << seconds << ":";
  if (millis < 10) timess << "0";
  timess << millis;

  timeDisplay->setText(timess.str());

  // Set player's gravity based on the direction they are facing
  Ogre::Vector3 ocam = app->mCameraNode->_getDerivedPosition();
  btVector3 facingDirection = player->getPosition() - btVector3(ocam[0], ocam[1], ocam[2]);
  facingDirection[1] = 0;
  facingDirection.normalize();

  btScalar yaw = btVector3(0,0,-1).angle(facingDirection);
  btQuaternion q((facingDirection[0] > 0 ? -yaw : yaw),0,0);
  q.normalize();

  if (gameEnded) {
    player->getBody()->setGravity(btVector3(0, 1000, 0));
  } else {
    player->getBody()->setGravity(app->mPhysics->getDynamicsWorld()->getGravity()
                                  .rotate(currTilt.getAxis(), -currTilt.getAngle())
                                  .rotate(currTilt.getAxis(), -currTilt.getAngle())
                                  .rotate(currTilt.getAxis(), -currTilt.getAngle())
                                  .rotate(q.getAxis(), q.getAngle()));
  }

  // Update Camera Position
  //comment out the lines below if you're building a level; also return false in mouseMoved.
  mCameraObj->update((Ogre::Vector3)player->getPosition(), elapsedTime);

  // This only works in this method, not from CameraObject. DONT ASK JUST ACCEPT
  app->mCameraNode->lookAt((Ogre::Vector3)player->getPosition() + Ogre::Vector3(0,250,0), Ogre::SceneNode::TS_WORLD);
  app->mCamera->lookAt((Ogre::Vector3)player->getPosition() + Ogre::Vector3(0,250,0));

  // Tilt Camera to simulate level tilt
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

  //Player *mPlayer = players[myId];
  //if (mPlayer) {
   // btVector3 pos = (mPlayer->pongMode ? mPlayer->getRacquet()->getPosition() : mPlayer->getNode()->getPosition());
   // mCamera->lookAt(pos[0], pos[1], pos[2]);
  //}

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
    app->paused = true;
    CEGUI::MouseCursor::getSingleton().show();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("PauseMenu"));
    app->Wmgr->getWindow("PauseMenu/Quit")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&HostPlayerActivity::ExitToMenu, this));
    app->Wmgr->getWindow("PauseMenu/Return")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&HostPlayerActivity::togglePauseMenu, this));
  } else {
    app->paused = false;
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
    togglePauseMenu();
    return true;
  }

  if (ceguiActive || menuActive) {
    CEGUI::System::getSingleton().injectKeyDown(arg.key);
    CEGUI::System::getSingleton().injectChar(arg.text);
  }

  switch(arg.key){
  case OIS::KC_D:
    tiltDest *= btQuaternion(0,0,-MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_A:
    tiltDest *= btQuaternion(0,0,MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_W:
    tiltDest *= btQuaternion(0,-MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_S:
    tiltDest *= btQuaternion(0,MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  default:
    return false;
  }

  return true;
}

//-------------------------------------------------------------------------------------

bool HostPlayerActivity::keyReleased( const OIS::KeyEvent &arg )
{
  if (menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectKeyUp(arg.key);
  }

  switch(arg.key){
  case OIS::KC_D:
    tiltDest *= btQuaternion(0,0,MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_A:
    tiltDest *= btQuaternion(0,0,-MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_W:
    tiltDest *= btQuaternion(0,MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_S:
    tiltDest *= btQuaternion(0,-MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  default:
    return false;
  }

  return true;
}

//-------------------------------------------------------------------------------------

bool HostPlayerActivity::mouseMoved( const OIS::MouseEvent &arg )
{
  if (menuActive || ceguiActive) {
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
  if (menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonDown(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------

bool HostPlayerActivity::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonUp(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}
