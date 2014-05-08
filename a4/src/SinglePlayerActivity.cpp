#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <termio.h>
#include <fcntl.h> 
#include <linux/input.h> 
#include <sstream>
#define STR_SIZE 256

#include "Interpolator.h"
#include "MenuActivity.h"
#include "SinglePlayerActivity.h"
#include "Leaderboard.h"
#include "OBAnimationManager.h"
#include "Sounds.h"
#include "SelectorHelper.h"

SinglePlayerActivity::SinglePlayerActivity(OgreBallApplication *app, const char* levelName, int mCharacter) : Activity(app) {
  MAX_TILT = .10; //Increasing this increases the maximum degree to which the level can rotate
  currTiltDelay = tiltDelay = 300;  // Increasing this increases the time it takes for the level to rotate
  lastTilt = btQuaternion(0,0,0);
  currTilt = btQuaternion(0,0,0);
  tiltDest = btQuaternion(0,0,0);
  currentLevelName = std::string(levelName, std::strlen(levelName));
  menuActive = false;
  ceguiActive = false;
  lives = 4;
  character = mCharacter;
  mGC = new GamecubeController("/dev/input/js0");
  mGC->init();
}

SinglePlayerActivity::~SinglePlayerActivity(void) {
  mGC->gc_close();
  closeActivity();
}

void SinglePlayerActivity::closeActivity(void) {
  delete player;
  delete mCameraObj;
}

bool SinglePlayerActivity::Retry( const CEGUI::EventArgs& e ) {
  CEGUI::MouseCursor::getSingleton().hide();
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("SinglePlayerHUD"));
  menuActive = false;
  ceguiActive = false;
  lives = 4;

  loadLevel(currentLevelName.c_str());
}

void SinglePlayerActivity::start(void) {
  Sounds::playBackground("media/OgreBall/sounds/StandardLevel.mp3");

  guiSheet = app->Wmgr->getWindow("SinglePlayerHUD");
  guiSheet->removeChildWindow(app->Wmgr->getWindow("ConsoleRoot"));
  CEGUI::System::getSingleton().setGUISheet(guiSheet);

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
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));

  pauseReturn->removeEvent(CEGUI::PushButton::EventClicked);
  pauseReturn
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::togglePauseMenu, this));


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
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));

  gwViewLeaderboard->removeEvent(CEGUI::PushButton::EventClicked);
  gwViewLeaderboard
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::ShowLeaderboard, this));

  gwNextLevel->removeEvent(CEGUI::PushButton::EventClicked);
  gwNextLevel
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::nextLevel, this));

  gameOverSheet = app->Wmgr->getWindow("GameOver");
  goGame = app->Wmgr->getWindow("GameOver/Game");
  goOver = app->Wmgr->getWindow("GameOver/Over");
  goRetry = app->Wmgr->getWindow("GameOver/Retry");
  goBackToMenu = app->Wmgr->getWindow("GameOver/BackToMenu");

  goBackToMenu->removeEvent(CEGUI::PushButton::EventClicked);
  goBackToMenu
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));

  goRetry->removeEvent(CEGUI::PushButton::EventClicked);
  goRetry
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::Retry, this));

  leaderboardWindow = app->Wmgr->getWindow("Leaderboard");
  leaderboardName = app->Wmgr->getWindow("Leaderboard/LevelName");
  leaderboardNextLevel = app->Wmgr->getWindow("Leaderboard/NextLevel");
  leaderboardBackToMenu = app->Wmgr->getWindow("Leaderboard/BackToMenu");

  leaderboardBackToMenu->removeEvent(CEGUI::PushButton::EventClicked);
  leaderboardBackToMenu
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::ExitToMenu, this));

  leaderboardNextLevel->removeEvent(CEGUI::PushButton::EventClicked);
  leaderboardNextLevel
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&SinglePlayerActivity::nextLevel, this));

  for (int i = 0; i < 10; i++) {
    std::stringstream ss;
    ss << "Leaderboard/" << i;
    leaderboardWindows[i] = app->Wmgr->getWindow(ss.str());
  }

  LEFT = false; 
  RIGHT = false; 
  FORWARD = false; 
  BACKWARD = false;
  mGC->reset();
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

  //Choose the correct mesh for the selected character
  char* playerChoice = SelectorHelper::CharacterToString(character); 

  player = new OgreBall(app->mSceneMgr, "player1", "player1", playerChoice, 0, app->mPhysics,
                        app->levelLoader->playerStartPositions[0], btVector3(1,1,1), btVector3(0,0,0),
                        16000.0f, 0.5f, btVector3(0,0,0), &app->levelLoader->playerStartRotations[0]);

  app->mCamera->setPosition(Ogre::Vector3(0,0,0));
  mCameraObj = new CameraObject(app->mCameraLookAtNode, app->mCameraNode,
                                (Ogre::Vector3)player->getPosition(), app->levelLoader->cameraStartPos);

  gameEnded = false;
  countdown = 2000;
  readyWindow->setAlpha(0.0);
  goWindow->setAlpha(0.0);

  menuActive = false;
  ceguiActive = false;
}

bool SinglePlayerActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);
  if (OgreBallApplication::debug) return false;
  return true;
}

bool SinglePlayerActivity::frameStarted( Ogre::Real elapsedTime ) {
  app->mPhysics->stepSimulation(elapsedTime);

  if (OgreBallApplication::debug) return true;

  
  if(mGC->connected){
  	mGC->capture();
	int MAX_TIMES = 10;	
	Ogre::Real sideTilt = .0075;
  	if(mGC->JOYSTICK_Y > 0 && !FORWARD){
		tiltDest *= btQuaternion(0, -MAX_TILT, 0);
		FORWARD = true;
	} else if(mGC->JOYSTICK_Y < 0 && !BACKWARD){
		tiltDest *= btQuaternion(0, MAX_TILT, 0);
		BACKWARD = true;
	} else if(mGC->JOYSTICK_Y == 0){ //if(mGC->JOYSTICK_Y == 0)
		if(FORWARD){
			tiltDest *= btQuaternion(0,MAX_TILT,0);
			FORWARD = false;
		}
		if(BACKWARD){
			tiltDest *= btQuaternion(0,-MAX_TILT,0);
			BACKWARD = false;
		}
	}
  	if(mGC->JOYSTICK_X > 0 && !LEFT){
		tiltDest *= btQuaternion(0, 0, -MAX_TILT);
		LEFT = true;
	} else if(mGC->JOYSTICK_X < 0 && !RIGHT){
		tiltDest *= btQuaternion(0, 0, MAX_TILT);
  		RIGHT = true;
	} else if(mGC->JOYSTICK_X == 0){ //if(mGC->JOYSTICK_X == 0)
		if(LEFT){
			tiltDest *= btQuaternion(0,0,MAX_TILT);
			LEFT = false;
		}
		if(RIGHT){
			tiltDest *= btQuaternion(0,0,-MAX_TILT);
			RIGHT = false;
		}
	
	}
	if(mGC->START_PRESSED){
		toggleGCPauseMenu();
	}
	lastTilt = currTilt; 
	currTiltDelay = 0; 
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
  } else if (countdown == -1) {
    btVector3 tweakedGrav = 1.8*app->mPhysics->getDynamicsWorld()->getGravity()
      .rotate(currTilt.getAxis(), -2.5*currTilt.getAngle())
      .rotate(q.getAxis(), q.getAngle());

    tweakedGrav[1] /= 1.5;
    player->getBody()->setGravity(tweakedGrav);

  }

  // Update Camera Position
  //comment out the lines below if you're building a level; also return false in mouseMoved.
  mCameraObj->update((Ogre::Vector3)player->getPosition(), elapsedTime);

  // This only works in this method, not from CameraObject. DONT ASK JUST ACCEPT
  //app->mCameraNode->lookAt((Ogre::Vector3)player->getPosition() + Ogre::Vector3(0,250,0), Ogre::SceneNode::TS_WORLD);
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
  return true;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::togglePauseMenu( const CEGUI::EventArgs& e ) {
  togglePauseMenu();
  return true;
}

void SinglePlayerActivity::toggleGCPauseMenu(){
	menuActive = !menuActive;
	if(menuActive){
		app->paused = true;
		CEGUI::MouseCursor::getSingleton().show();
		CEGUI::System::getSingleton().setGUISheet(pauseMenuSheet);
	//	mGC->blockingCapture();
	//	if(mGC->START_PRESSED){
	//		app->paused = false;
	////		CEGUI::MouseCursor::getSingleton().hide();
	//		CEGUI::System::getSingleton().setGUISheet(guiSheet);
	//	}
	} else {
		app->paused = false;
		CEGUI::MouseCursor::getSingleton().hide();
		CEGUI::System::getSingleton().setGUISheet(guiSheet);
	}
}

void SinglePlayerActivity::togglePauseMenu( ) {
  menuActive = !menuActive;
  if (menuActive) {
    app->paused = true;
    CEGUI::MouseCursor::getSingleton().show();
    CEGUI::System::getSingleton().setGUISheet(pauseMenuSheet);

  } else {
    app->paused = false;
    CEGUI::MouseCursor::getSingleton().hide();
    CEGUI::System::getSingleton().setGUISheet(guiSheet);
  }
}

bool SinglePlayerActivity::nextLevel( const CEGUI::EventArgs& e ) {
  LevelLoader *loader = LevelLoader::getSingleton();
  int nextLevel = loader->mCurrLevelID;

  nextLevel++;
  if(nextLevel >= loader->levelNames.size())
    nextLevel = 0;

  loadLevel((loader->levelNames[nextLevel]).c_str());
  CEGUI::MouseCursor::getSingleton().hide();
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("SinglePlayerHUD"));

  return true;
}

bool SinglePlayerActivity::ShowLeaderboard( const CEGUI::EventArgs& e ) {
  CEGUI::System::getSingleton().setGUISheet(leaderboardWindow);

  for (int i = 0; i < 10; i++)
    leaderboardWindows[i]->setAlpha(0.0);

  Leaderboard leaderboard = Leaderboard::findLeaderboard(currentLevelName.c_str());
  leaderboardName->setText(currentLevelName.c_str());
  OBAnimationManager::startAnimation("SpinPopup", leaderboardName);
  OBAnimationManager::startAnimation("SpinPopup", leaderboardNextLevel, 0.5);
  OBAnimationManager::startAnimation("SpinPopup", leaderboardBackToMenu, 0.5);

  multimap<double, LeaderboardEntry, greater<double> > highscores = leaderboard.getHighscores();
  multimap<double, LeaderboardEntry>::iterator iter;

  int i = 0;
  for (iter = highscores.begin(); iter != highscores.end(); iter++) {
    LeaderboardEntry entry = iter->second;
    std::stringstream ss;

    size_t namelen = entry.name.length();
    ss << std::left << setw(55-namelen) << entry.name <<
      setw(15) << entry.score <<
      setw(15) << entry.getTimeTaken() <<
      setw(25) << entry.getTimeEntered();

    leaderboardWindows[i]->setText(ss.str());

    OBAnimationManager::startAnimation("FadeInFromLeft", leaderboardWindows[i], 1.0, 1.0 + 0.2f*i);
    i++;
  }
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
  gwNameEditText->setAlpha(0.0);
  gwSubmitHighscore->setAlpha(0.0);

  OBAnimationManager::startAnimation("SpinPopup", gwGoal);
  OBAnimationManager::startAnimation("SpinPopup", gwNextLevel, 0.5);
  OBAnimationManager::startAnimation("SpinPopup", gwBackToMenu, 0.5);
  OBAnimationManager::startAnimation("SpinPopup", gwViewLeaderboard, 0.35);

  int totalScore = score + timeLeft + (collectibles == app->levelLoader->numCollectibles ? 10000 : 0);

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
    OBAnimationManager::startAnimation("SpinPopup", gwHighscore, 1.0, 2.2f);
    pulsatingAnim = OBAnimationManager::startAnimation("StartButtonPulsating", gwHighscore, 0.7, 3.6f);
    OBAnimationManager::startAnimation("Hide", gwGoal, 1.0, 2.0f);

    // Show window asking for name w/ add to leaderboard button to save
    OBAnimationManager::startAnimation("FadeInFromLeft", gwNameEditText, 1.0, 3.2f);
    OBAnimationManager::startAnimation("FadeInFromLeft", gwSubmitHighscore, 1.0, 3.2f);

    gwSubmitHighscore->setText("Add to Leaderboards");

    gwSubmitHighscore->removeEvent(CEGUI::PushButton::EventClicked);
    gwSubmitHighscore
      ->subscribeEvent(CEGUI::PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&SinglePlayerActivity::HandleHighscoreSubmitted, this));
  }
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
}

bool SinglePlayerActivity::HandleHighscoreSubmitted( const CEGUI::EventArgs &e ) {
  CEGUI::String name = gwNameEditText->getText();
  if (name.length() == 0) return true;

  int totalScore = score + timeLeft + (collectibles == app->levelLoader->numCollectibles ? 10000 : 0);

  Leaderboard leaderboard = Leaderboard::findLeaderboard(currentLevelName.c_str());
  leaderboard.addHighscore(name.c_str(), totalScore, 60000 - timeLeft);
  leaderboard.saveToFile();

  gwNameEditText->setAlpha(0.0);
  gwSubmitHighscore->setText("Submitted!");
  OBAnimationManager::startAnimation("SpinPopup", gwSubmitHighscore);
  gwSubmitHighscore->removeEvent(CEGUI::PushButton::EventClicked);
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

  //printf("tiltDest (%f,%f,%f)\n", tiltDest[0], tiltDest[1], tiltDest[2]);
  switch(arg.key){
  case OIS::KC_D:
 //   printf("tilting right\n");
    tiltDest *= btQuaternion(0,0,-MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_A:
  //  printf("tilting left\n");
    tiltDest *= btQuaternion(0,0,MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_W:
  //  printf("tilting forward\n");
    tiltDest *= btQuaternion(0,-MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_S:
  //:  printf("tilting backwards\n");
    tiltDest *= btQuaternion(0,MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_R:
    break;
  default:
    return false;
  }

  //printf("tiltDest after (%f,%f,%f)\n", tiltDest[0], tiltDest[1], tiltDest[2]);
  return true;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::keyReleased( const OIS::KeyEvent &arg )
{
  if (menuActive || ceguiActive) {
    CEGUI::System::getSingleton().injectKeyUp(arg.key);
  }

  if (OgreBallApplication::debug) return false;


  //printf("tiltDest (%f,%f,%f)\n", tiltDest[0], tiltDest[1], tiltDest[2]);
  switch(arg.key){
  case OIS::KC_D:
    //printf("untilting right\n");
    tiltDest *= btQuaternion(0,0,MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_A:
    //printf("untilting left\n");
    tiltDest *= btQuaternion(0,0,-MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_W:
  // printf("untilting forwards\n");
    tiltDest *= btQuaternion(0,MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_S:
    //printf("untilting backwards\n");
    tiltDest *= btQuaternion(0,-MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_R:
    if (lives > 0 && !gameEnded && countdown == -1) {
      lives--;
      OBAnimationManager::startAnimation("SpinPopup", livesDisplay);
      loadLevel(currentLevelName.c_str());
      break;
    }
  default:
    return false;
  }

  //printf("tiltDest after (%f,%f,%f)\n", tiltDest[0], tiltDest[1], tiltDest[2]);
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
