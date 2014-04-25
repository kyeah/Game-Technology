#include "Interpolator.h"
#include "MenuActivity.h"
#include "SinglePlayerActivity.h"
#include "Leaderboard.h"

SinglePlayerActivity::SinglePlayerActivity(OgreBallApplication *app, const char* levelName) : Activity(app) {
  MAX_TILT = .10; //Increasing this increases the maximum degree to which the level can rotate
  currTiltDelay = tiltDelay = 300;  // Increasing this increases the time it takes for the level to rotate
  lastTilt = btQuaternion(0,0,0);
  currTilt = btQuaternion(0,0,0);
  tiltDest = btQuaternion(0,0,0);
  currentLevelName = std::string(levelName, std::strlen(levelName));
  menuActive = false;
  ceguiActive = false;
  lives = 4;
  score = 0;
}

SinglePlayerActivity::~SinglePlayerActivity(void) {
  close();
}

void SinglePlayerActivity::close(void) {
  delete player;
  delete mCameraObj;

}

void SinglePlayerActivity::start(void) {
  guiSheet = app->Wmgr->getWindow("SinglePlayerHUD");
  CEGUI::System::getSingleton().setGUISheet(guiSheet);
  scoreDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Score");
  livesDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Lives");
  collectDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Collectibles");
  timeDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Timer");
  levelDisplay = app->Wmgr->getWindow("SinglePlayerHUD/Level");

  loadLevel(currentLevelName.c_str());
}

void SinglePlayerActivity::loadLevel(const char* name) {
  currentLevelName = std::string(name, std::strlen(name));
  app->destroyAllEntitiesAndNodes();
  app->levelLoader->loadLevel(name);

  levelDisplay->setText(currentLevelName.c_str());

  timeLeft = 60000;  // TODO: Should get timeLeft from level script
  collectibles = 0;  // TODO: Get total number of collectibles when loading level

  player = new OgreBall(app->mSceneMgr, "player1", "player1", "penguin.mesh", 0, app->mPhysics,
                        app->levelLoader->playerStartPositions[0], btVector3(1,1,1), btVector3(0,0,0),
                        16000.0f, 0.5f, btVector3(0,0,0), &app->levelLoader->playerStartRotations[0]);

  app->mCamera->setPosition(Ogre::Vector3(0,0,0));
  mCameraObj = new CameraObject(app->mCameraLookAtNode, app->mCameraNode,
                                (Ogre::Vector3)player->getPosition(), app->levelLoader->cameraStartPos);

  gameEnded = false;
}

bool SinglePlayerActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);
  if (OgreBallApplication::debug) return false;
  return true;
}

bool SinglePlayerActivity::frameStarted( Ogre::Real elapsedTime ) {
  if (OgreBallApplication::debug) return true;

  timeLeft = std::max(timeLeft - elapsedTime, 0.0f);
  if (!gameEnded) {
    if (timeLeft == 0.0f) {
      handleGameOver();
    } else if (app->levelLoader->fallCutoff > player->getPosition()[1]) {
      lives--;
      if (lives < 0) {
        handleGameOver();
      } else {
        CEGUI::AnimationManager *mgr = CEGUI::AnimationManager::getSingletonPtr();
        CEGUI::AnimationInstance* instance = mgr->instantiateAnimation(mgr->getAnimation("SpinPopup"));
        instance->setTargetWindow(app->Wmgr->getWindow("SinglePlayerHUD/Lives"));
        instance->start();
        loadLevel(currentLevelName.c_str());
        return true;
      }
    }
  }

  currTilt = Interpolator::interpQuat(currTiltDelay, elapsedTime, tiltDelay,
                                      lastTilt, tiltDest);

  // Update HUD
  std::stringstream sst;
  sst << "SCORE: " << score;
  scoreDisplay->setText(sst.str());

  std:: stringstream livesSS;
  livesSS << lives << (lives != 1 ? " Lives" : " Life");
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

      app->Wmgr->getWindow("PauseMenu/Quit")->removeEvent(CEGUI::PushButton::EventClicked);
      app->Wmgr->getWindow("PauseMenu/Quit")
        ->subscribeEvent(CEGUI::PushButton::EventClicked,
                         CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));

      app->Wmgr->getWindow("PauseMenu/Return")->removeEvent(CEGUI::PushButton::EventClicked);
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
    gameEnded = true;

    CEGUI::MouseCursor::getSingleton().show();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("GameWon"));

    CEGUI::AnimationManager *mgr = CEGUI::AnimationManager::getSingletonPtr();
    CEGUI::AnimationInstance* instance = mgr->instantiateAnimation(mgr->getAnimation("SpinPopup"));
    instance->setTargetWindow(app->Wmgr->getWindow("GameWon/Goal"));
    instance->start();

    instance = mgr->instantiateAnimation(mgr->getAnimation("SpinPopup"));
    instance->setTargetWindow(app->Wmgr->getWindow("GameWon/NextLevel"));
    instance->setSpeed(0.5);
    instance->start();

    instance = mgr->instantiateAnimation(mgr->getAnimation("SpinPopup"));
    instance->setTargetWindow(app->Wmgr->getWindow("GameWon/BackToMenu"));
    instance->setSpeed(0.5);
    instance->start();

    app->Wmgr->getWindow("GameWon/BackToMenu")->removeEvent(CEGUI::PushButton::EventClicked);
    app->Wmgr->getWindow("GameWon/BackToMenu")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));

    /*  app->Wmgr->getWindow("GameWon/NextLevel")
        ->subscribeEvent(CEGUI::PushButton::EventClicked,
        CEGUI::Event::Subscriber(&SinglePlayerActivity::nextLevel, this));*/
  }

  void SinglePlayerActivity::handleGameOver() {
    ceguiActive = true;
    gameEnded = true;

    CEGUI::MouseCursor::getSingleton().show();
    CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("GameOver"));

    CEGUI::AnimationManager *mgr = CEGUI::AnimationManager::getSingletonPtr();
    CEGUI::AnimationInstance* instance = mgr->instantiateAnimation(mgr->getAnimation("SpinPopup"));
    instance->setTargetWindow(app->Wmgr->getWindow("GameOver/Game"));
    instance->setSpeed(0.4);
    instance->start();

    instance = mgr->instantiateAnimation(mgr->getAnimation("SpinPopup"));
    instance->setTargetWindow(app->Wmgr->getWindow("GameOver/Over"));
    instance->setSpeed(0.25);
    instance->start();

    instance = mgr->instantiateAnimation(mgr->getAnimation("SpinPopup"));
    instance->setTargetWindow(app->Wmgr->getWindow("GameOver/Retry"));
    instance->setSpeed(0.2);
    instance->start();

    instance = mgr->instantiateAnimation(mgr->getAnimation("SpinPopup"));
    instance->setTargetWindow(app->Wmgr->getWindow("GameOver/BackToMenu"));
    instance->setSpeed(0.2);
    instance->start();

    app->Wmgr->getWindow("GameOver/BackToMenu")->removeEvent(CEGUI::PushButton::EventClicked);
    app->Wmgr->getWindow("GameOver/BackToMenu")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));
  }

  //-------------------------------------------------------------------------------------

  bool SinglePlayerActivity::keyPressed( const OIS::KeyEvent &arg )
  {
    if (arg.key == OIS::KC_ESCAPE) {
      if (!gameEnded) togglePauseMenu();
      return true;
    }

    if (ceguiActive || menuActive) {
      CEGUI::System::getSingleton().injectKeyDown(arg.key);
      CEGUI::System::getSingleton().injectChar(arg.text);
    }

    if (OgreBallApplication::debug) return false;

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

    if (OgreBallApplication::debug) return false;

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

    if (OgreBallApplication::debug) return false;
    else return true;
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
