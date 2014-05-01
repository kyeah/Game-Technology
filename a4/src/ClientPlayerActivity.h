#pragma once

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include "Activity.h"
#include "CameraObject.h"
#include "GameObjectDescription.h"
#include "common.h"

class ClientPlayerActivity : public Activity {
 public:
  ClientPlayerActivity(OgreBallApplication *app, int id, const char* levelName);
  virtual ~ClientPlayerActivity(void);
  virtual void close(void);

  void start(void);
  virtual bool frameStarted( Ogre::Real elapsedTime );
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

  virtual bool keyPressed( const OIS::KeyEvent &arg );
  virtual bool keyReleased( const OIS::KeyEvent &arg );

  virtual bool mouseMoved( const OIS::MouseEvent &arg );
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

  void handleLobbyState();
  virtual void handleGameEnd();

  void loadLevel( const char* name );
  bool ExitToMenu( const CEGUI::EventArgs& e );
  bool togglePauseMenu( const CEGUI::EventArgs& e );
  bool waitForHosts();
  void togglePauseMenu();

  Player* addPlayer(int userID, const char* name);

  std::string currentLevelName;

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

  CEGUI::Window *lobbySheet, *lobbySelectLevel, *lobbySelectCharacter,
    *lobbyLeave, *lobbyStart;

  CEGUI::Window *lobbyPlayerWindows[4];

  bool menuActive, ceguiActive, gameEnded, levelLoaded,
    waitingScreenLoaded, readyToLoadLevel, inGame, chatFocus;

  int myId;
  int countdown;

  // Game State Variables
  OgreBall *player;
  float timeLeft;  // In millis
  int lives;
};
