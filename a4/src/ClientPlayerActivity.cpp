#include "Interpolator.h"
#include "MenuActivity.h"
#include "ClientPlayerActivity.h"
#include "common.h"
#include "Networking.h"

ClientPlayerActivity::ClientPlayerActivity(OgreBallApplication *app, const char* hostName) : Activity(app) {
  MAX_TILT = .10; //Increasing this increases the maximum degree to which the level can rotate
  currTiltDelay = tiltDelay = 300;  // Increasing this increases the time it takes for the level to rotate
  lastTilt = btQuaternion(0,0,0);
  currTilt = btQuaternion(0,0,0);
  tiltDest = btQuaternion(0,0,0);
  currentLevelName = "undefined";
  levelLoaded = false;
  waitingScreenLoaded = false;
  readyToLoadLevel = false;
  menuActive = false;
  ceguiActive = false;
  lives = 10;
  Networking::clientConnect(&myId,(char*) hostName);
}

ClientPlayerActivity::~ClientPlayerActivity(void) {
  Networking::Close();
  close();
}

void ClientPlayerActivity::close(void) {
  delete player;
  delete mCameraObj;

}

void ClientPlayerActivity::start(void) {
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("SinglePlayerHUD"));
  scoreDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Score");
  livesDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Lives");
  collectDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Collectibles");
  timeDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Timer");
  levelDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Level");

//  loadLevel(currentLevelName);
}

void ClientPlayerActivity::loadLevel(const char* name) {


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

bool ClientPlayerActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  return true;
}

bool ClientPlayerActivity::frameStarted( Ogre::Real elapsedTime ) {

  
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

  if(!levelLoaded){
	if(!waitForHosts())
		return true;
  }

  timeLeft = std::max(timeLeft - elapsedTime, 0.0f);
  currTilt = Interpolator::interpQuat(currTiltDelay, elapsedTime, tiltDelay,
                                      lastTilt, tiltDest);

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
    app->paused = true;
    CEGUI::MouseCursor::getSingleton().show();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("PauseMenu"));
    app->Wmgr->getWindow("PauseMenu/Quit")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&ClientPlayerActivity::ExitToMenu, this));
    app->Wmgr->getWindow("PauseMenu/Return")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&ClientPlayerActivity::togglePauseMenu, this));
  } else {
    app->paused = false;
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

bool ClientPlayerActivity::keyReleased( const OIS::KeyEvent &arg )
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

bool ClientPlayerActivity::mouseMoved( const OIS::MouseEvent &arg )
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

bool ClientPlayerActivity::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonDown(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------

bool ClientPlayerActivity::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonUp(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}
