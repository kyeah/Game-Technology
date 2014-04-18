#include "Interpolator.h"
#include "MenuActivity.h"
#include "SinglePlayerActivity.h"


SinglePlayerActivity::SinglePlayerActivity(OgreBallApplication *app, const char* levelName) : Activity(app) {
  MAX_TILT = .10; //Increasing this increases the maximum degree to which the level can rotate
  currTiltDelay = tiltDelay = 300;  // Increasing this increases the time it takes for the level to rotate
  lastTilt = btQuaternion(0,0,0);
  currTilt = btQuaternion(0,0,0);
  tiltDest = btQuaternion(0,0,0);
  currentLevelName = levelName;
  menuActive = false;
  ceguiActive = false;
  lives = 10;
}

SinglePlayerActivity::~SinglePlayerActivity(void) {
}

void SinglePlayerActivity::start(void) {
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("SinglePlayerHUD"));
  scoreDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Score");
  livesDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Lives");
  collectDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Collectibles");
  timeDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Timer");
  levelDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Level");

  loadLevel(currentLevelName);
}

void SinglePlayerActivity::loadLevel(const char* name) {

  app->destroyAllEntitiesAndNodes();
  app->levelLoader->loadLevel(name);
  app->mSceneMgr->setSkyDome(true,"Examples/CloudySky", 5, 8);

  mCameraObj = new CameraObject(app->mCamera);
  levelDisplay->setText(name);

  timeLeft = 60000;  // TODO: Should get timeLeft from level script
  collectibles = 0;  // TODO: Get total number of collectibles when loading level
  score = 0;

  player = new OgreBall(app->mSceneMgr, "player1", "player1", "penguin.mesh", 0, app->mPhysics,
                        app->levelLoader->playerStartPositions[0], btVector3(1,1,1), btVector3(0,0,0),
                        16000.0f, 1.0f, btVector3(0,0,0), &app->levelLoader->playerStartRotations[0]);
}

bool SinglePlayerActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  return true;
}

bool SinglePlayerActivity::frameStarted( Ogre::Real elapsedTime ) {
  timeLeft = std::max(timeLeft - elapsedTime, 0.0f);
  currTilt = Interpolator::interpQuat(currTiltDelay, elapsedTime, tiltDelay,
                                      lastTilt, tiltDest);

  player->getBody()->setGravity(app->mPhysics->getDynamicsWorld()->getGravity()
                                .rotate(currTilt.getAxis(), -currTilt.getAngle()));
  

  std::stringstream sst;
  sst << "SCORE: " << score;
  scoreDisplay->setText(sst.str());

  livesDisplay->setText(std::to_string(lives));
  collectDisplay->setText(std::to_string(collectibles));

  std::stringstream timess;
  int seconds = std::round(timeLeft/1000);
  int millis = std::min((float)99.0, std::round(fmod(timeLeft,1000)/10));

  timess << seconds << ":";
  if (millis < 10) timess << "0";
  timess << millis;

  timeDisplay->setText(timess.str());

  if(!mCameraObj->previousPosIsSet)
       	mCameraObj->setPreviousPosition((Ogre::Vector3)player->getPosition());
  if(!mCameraObj->fixedDist || (app->levelLoader->cameraStartPos != mCameraObj->cameraStartPosition))
       	mCameraObj->setFixedDistance((Ogre::Vector3)player->getPosition(), app->levelLoader->cameraStartPos);
  mCameraObj->update((Ogre::Vector3)player->getPosition(), elapsedTime);

  // More magic stuff here to make the level look like it's rotating
  /*  app->mCamera->setOrientation(Ogre::Quaternion(currTilt.w(),
      -currTilt.x(),
      -currTilt.y(),
      -currTilt.z()));*/
  return true;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::togglePauseMenu( const CEGUI::EventArgs& e ) {
  togglePauseMenu();
  return true;
}

void SinglePlayerActivity::togglePauseMenu( ) {
  menuActive = !menuActive;
  if (menuActive) {
    app->paused = true;
    CEGUI::MouseCursor::getSingleton().show();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("PauseMenu"));
    app->Wmgr->getWindow("PauseMenu/Quit")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));
    app->Wmgr->getWindow("PauseMenu/Return")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&SinglePlayerActivity::togglePauseMenu, this));
  } else {
    app->paused = false;
    CEGUI::MouseCursor::getSingleton().hide();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("SinglePlayerHUD"));
  }
}

bool SinglePlayerActivity::ExitToMenu( const CEGUI::EventArgs& e ) {
  app->switchActivity(new MenuActivity(app));
  return true;
}

void SinglePlayerActivity::handleGameEnd() {
  ceguiActive = true;
  CEGUI::MouseCursor::getSingleton().show();
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("GameWon"));

  app->Wmgr->getWindow("GameWon/BackToMenu")
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));
  /*  app->Wmgr->getWindow("GameWon/NextLevel")
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&SinglePlayerActivity::nextLevel, this));*/
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::keyPressed( const OIS::KeyEvent &arg )
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

bool SinglePlayerActivity::keyReleased( const OIS::KeyEvent &arg )
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

bool SinglePlayerActivity::mouseMoved( const OIS::MouseEvent &arg )
{
  if (menuActive || ceguiActive) {
    CEGUI::System &sys = CEGUI::System::getSingleton();
    sys.injectMouseMove(arg.state.X.rel, arg.state.Y.rel);
    // Scroll wheel.
    if (arg.state.Z.rel)
      sys.injectMouseWheelChange(arg.state.Z.rel / 120.0f);
    return true;
  }

  return false;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonDown(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  if (menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectMouseButtonUp(OgreBallApplication::convertButton(id));
    return true;
  }
  return false;
}
