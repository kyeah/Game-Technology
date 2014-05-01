#pragma once

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include "Activity.h"
#include "CameraObject.h"
#include "GameObjectDescription.h"

class SinglePlayerActivity : public Activity {
 public:
  SinglePlayerActivity(OgreBallApplication *app, const char* levelName, int mCharacter=0);
  virtual ~SinglePlayerActivity(void);
  virtual void close(void);

  void start(void);
  virtual bool frameStarted( Ogre::Real elapsedTime );
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

  virtual bool keyPressed( const OIS::KeyEvent &arg );
  virtual bool keyReleased( const OIS::KeyEvent &arg );

  virtual bool mouseMoved( const OIS::MouseEvent &arg );
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual void handleGameEnd();

  void handleGameOver();
  bool HandleHighscoreSubmitted( const CEGUI::EventArgs& e );

  void loadLevel( const char* name );
  bool nextLevel( const CEGUI::EventArgs& e );
  bool Retry( const CEGUI::EventArgs& e );
  bool ShowLeaderboard( const CEGUI::EventArgs& e);
  bool ExitToMenu( const CEGUI::EventArgs& e );
  bool togglePauseMenu( const CEGUI::EventArgs& e );
  void togglePauseMenu();

  std::string currentLevelName;

  const static int CHARACTER_PENGUIN = 0;
  const static int CHARACTER_OGRE = 1;
  const static int CHARACTER_NINJA = 2;

  // User Input Variables
  btScalar MAX_TILT;
  btQuaternion lastTilt, currTilt, tiltDest;
  float currTiltDelay, tiltDelay;
  OgreBall* mOgreBall;  
  CameraObject* mCameraObj;

  // Menu Variables
  CEGUI::Window *guiSheet, *scoreDisplay, *timeDisplay, 
    *collectDisplay, *livesDisplay, *levelDisplay;

  CEGUI::Window *readyWindow, *goWindow;

  CEGUI::Window *pauseMenuSheet, *pauseQuit, *pauseReturn;
  CEGUI::Window *gameWonSheet, *gwGoal, *gwNextLevel, *gwBackToMenu,
    *gwViewLeaderboard, *gwTimeTaken, *gwCollectibles, *gwBonus, *gwScore, *gwHighscore,
    *gwNameEditText, *gwSubmitHighscore;

  CEGUI::Window *gameOverSheet, *goGame, *goOver, *goRetry, *goBackToMenu;

  CEGUI::Window* leaderboardWindow, *leaderboardName, *leaderboardNextLevel, *leaderboardBackToMenu;
  CEGUI::Window* leaderboardWindows[10];

  bool menuActive;
  bool ceguiActive;
  bool gameEnded;

  // Game State Variables
  OgreBall *player;
  float timeLeft;  // In millis
  int lives;
  int character;
  int countdown;

};
