#include "Interpolator.h"
#include "MenuActivity.h"
#include "SinglePlayerActivity.h"
#include "Leaderboard.h"
#include "OBAnimationManager.h"

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

  pauseMenuSheet = app->Wmgr->getWindow("PauseMenu");
  pauseQuit = app->Wmgr->getWindow("PauseMenu/Quit");
  pauseReturn = app->Wmgr->getWindow("PauseMenu/Return");

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

  gameOverSheet = app->Wmgr->getWindow("GameOver");
  goGame = app->Wmgr->getWindow("GameOver/Game");
  goOver = app->Wmgr->getWindow("GameOver/Over");
  goRetry = app->Wmgr->getWindow("GameOver/Retry");
  goBackToMenu = app->Wmgr->getWindow("GameOver/BackToMenu");

  leaderboardWindow = app->Wmgr->getWindow("Leaderboard");
  leaderboardName = app->Wmgr->getWindow("Leaderboard/LevelName");
  leaderboardNextLevel = app->Wmgr->getWindow("Leaderboard/NextLevel");
  leaderboardBackToMenu = app->Wmgr->getWindow("Leaderboard/BackToMenu");

  for (int i = 0; i < 10; i++) {
    std::stringstream ss;
    ss << "Leaderboard/" << i;
    leaderboardWindows[i] = app->Wmgr->getWindow(ss.str());
  }

  loadLevel(currentLevelName.c_str());
}

void SinglePlayerActivity::loadLevel(const char* name) {
  currentLevelName = std::string(name, std::strlen(name));
  app->destroyAllEntitiesAndNodes();
  app->levelLoader->loadLevel(name);

  levelDisplay->setText(currentLevelName.c_str());

  timeLeft = 60000;  // TODO: Should get timeLeft from level script
  collectibles = 0;
  score = 0;

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
        OBAnimationManager::startAnimation("SpinPopup", livesDisplay);
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
    CEGUI::System::getSingleton().setGUISheet(pauseMenuSheet);

    pauseQuit->removeEvent(CEGUI::PushButton::EventClicked);
    pauseQuit
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));

    pauseReturn->removeEvent(CEGUI::PushButton::EventClicked);
    pauseReturn
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&SinglePlayerActivity::togglePauseMenu, this));
  } else {
    app->paused = false;
    CEGUI::MouseCursor::getSingleton().hide();
    CEGUI::System::getSingleton().setGUISheet(guiSheet);
  }
}

bool SinglePlayerActivity::ShowLeaderboard( const CEGUI::EventArgs& e ) {
  CEGUI::System::getSingleton().setGUISheet(leaderboardWindow);

  leaderboardBackToMenu->removeEvent(CEGUI::PushButton::EventClicked);
  leaderboardBackToMenu
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));

  /*
  leaderboardNextLevel->removeEvent(CEGUI::PushButton::EventClicked);
  leaderboardNextLevel
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::SOMETINH, this));
  */
  for (int i = 0; i < 10; i++)
    leaderboardWindows[i]->setAlpha(0.0);

  Leaderboard leaderboard = Leaderboard::findLeaderboard(currentLevelName.c_str());
  leaderboardName->setText(currentLevelName.c_str());
  OBAnimationManager::startAnimation("SpinPopup", leaderboardName);
  OBAnimationManager::startAnimation("SpinPopup", leaderboardNextLevel, 0.5);
  OBAnimationManager::startAnimation("SpinPopup", leaderboardBackToMenu, 0.5);

  //int totalScore = score + timeLeft + (collectibles == app->levelLoader->numCollectibles ? 10000 : 0);
  //  leaderboard.addHighscore("KKKKKKK", totalScore, 60000 - timeLeft);

  multimap<double, LeaderboardEntry, greater<double> > highscores = leaderboard.getHighscores();
  multimap<double, LeaderboardEntry>::iterator iter;

  int i = 0;
  for (iter = highscores.begin(); iter != highscores.end(); iter++) {
    LeaderboardEntry entry = iter->second;
    std::stringstream ss;

    ss << std::left << setw(30) << entry.name <<
      setw(15) << entry.score <<
      setw(15) << entry.getTimeTaken() <<
      setw(25) << entry.getTimeEntered();

    leaderboardWindows[i]->setText(ss.str());

    OBAnimationManager::startAnimation("FadeInFromLeft", leaderboardWindows[i], 1.0, 1.0 + 0.2f*i);
    i++;
  }

  leaderboard.saveToFile();
}

bool SinglePlayerActivity::ExitToMenu( const CEGUI::EventArgs& e ) {
  app->switchActivity(new MenuActivity(app));
  return true;
}

void SinglePlayerActivity::handleGameEnd() {
  if (gameEnded) return;

  static CEGUI::AnimationInstance *pulsatingAnim = 0;
  if (pulsatingAnim) {
    pulsatingAnim->stop();
    CEGUI::AnimationManager::getSingleton().destroyAnimationInstance(pulsatingAnim);    
    pulsatingAnim = 0;
  }

  ceguiActive = true;
  gameEnded = true;

  CEGUI::MouseCursor::getSingleton().show();
  CEGUI::System::getSingleton().setGUISheet(gameWonSheet);

  gwGoal->setAlpha(0.0);
  gwNextLevel->setAlpha(0.0);
  gwBackToMenu->setAlpha(0.0);
  gwViewLeaderboard->setAlpha(0.0);
  gwHighscore->setAlpha(0.0);

  OBAnimationManager::startAnimation("SpinPopup", gwGoal);
  OBAnimationManager::startAnimation("SpinPopup", gwNextLevel, 0.5);
  OBAnimationManager::startAnimation("SpinPopup", gwBackToMenu, 0.5);
  OBAnimationManager::startAnimation("SpinPopup", gwViewLeaderboard, 0.35);

  gwBackToMenu->removeEvent(CEGUI::PushButton::EventClicked);
  gwBackToMenu
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));

  gwViewLeaderboard->removeEvent(CEGUI::PushButton::EventClicked);
  gwViewLeaderboard
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::ShowLeaderboard, this));

  int totalScore = score + timeLeft + (collectibles == app->levelLoader->numCollectibles ? 10000 : 0);
  //  leaderboard.addHighscore("KKKKKKK", totalScore, 60000 - timeLeft);

  std::stringstream timess;
  int seconds = std::round((60000-timeLeft)/1000);
  int millis = std::min((float)99.0, (float)std::round(fmod((60000-timeLeft),1000)/10));

  timess << std::left << setw(20) << "Time Taken: " << std::left << seconds << ":";
  if (millis < 10) timess << "0";
  timess << millis;

  gwTimeTaken->setText(timess.str());
  
  std::stringstream css;
  css << std::left << setw(21) << "Collectibles: " << collectibles << "/" << app->levelLoader->numCollectibles;

  gwCollectibles->setText(css.str());
  
  std::stringstream bss;
  bss << std::left << setw(25) << "Bonus: " << setw(30) <<
    (collectibles == app->levelLoader->numCollectibles
     ? "Got all Collectibles => 10,000"
     : "---");

  gwBonus->setText(bss.str());

  std::stringstream scoress;
  scoress << std::left << setw(21) << "Final Score: " << totalScore;

  gwScore->setText(scoress.str());

  gwTimeTaken->setAlpha(0.0);
  gwCollectibles->setAlpha(0.0);
  gwBonus->setAlpha(0.0);
  gwScore->setAlpha(0.0);

  OBAnimationManager::startAnimation("FadeInFromLeft", gwTimeTaken, 1.0, 1.0);
  OBAnimationManager::startAnimation("FadeInFromLeft", gwCollectibles, 1.0, 1.2);
  OBAnimationManager::startAnimation("FadeInFromLeft", gwBonus, 1.0, 1.4);
  OBAnimationManager::startAnimation("FadeInFromLeft", gwScore, 1.0, 1.6);

  Leaderboard leaderboard = Leaderboard::findLeaderboard(currentLevelName.c_str());
  if (leaderboard.isHighscore(totalScore)) {
    OBAnimationManager::startAnimation("SpinPopup", gwHighscore, 1.0, 2.0f);
    pulsatingAnim = OBAnimationManager::startAnimation("StartButtonPulsating", gwHighscore, 0.7, 3.4f);
    OBAnimationManager::startAnimation("Hide", gwGoal, 1.0, 2.0f);

    // TODO: Show window asking for name w/ add to leaderboard button to save
    leaderboard.addHighscore("KKKKKKK", totalScore, 60000 - timeLeft);
    leaderboard.saveToFile();
  }

  /*  app->Wmgr->getWindow("GameWon/NextLevel")
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&SinglePlayerActivity::nextLevel, this));*/
}

void SinglePlayerActivity::handleGameOver() {
  ceguiActive = true;
  gameEnded = true;

  goGame->setAlpha(0.0);
  goOver->setAlpha(0.0);
  goRetry->setAlpha(0.0);
  goBackToMenu->setAlpha(0.0);

  CEGUI::MouseCursor::getSingleton().show();
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("GameOver"));

  OBAnimationManager::startAnimation("SpinPopup", goGame, 0.4);
  OBAnimationManager::startAnimation("SpinPopup", goOver, 0.25);
  OBAnimationManager::startAnimation("SpinPopup", goRetry, 0.2);
  OBAnimationManager::startAnimation("SpinPopup", goBackToMenu, 0.2);

  goBackToMenu->removeEvent(CEGUI::PushButton::EventClicked);
  goBackToMenu
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
