#pragma once

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include "Activity.h"
#include "CameraObject.h"
#include "GameObjectDescription.h"
#include "Networking.h"
#include "common.h"

class HostPlayerActivity : public Activity {
 public:
  HostPlayerActivity(OgreBallApplication *app, const char* levelName);
  virtual ~HostPlayerActivity(void);
  virtual void close(void);

  void start(void);
  virtual bool frameStarted( Ogre::Real elapsedTime );
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

  virtual bool keyPressed( const OIS::KeyEvent &arg );
  virtual bool keyReleased( const OIS::KeyEvent &arg );

  virtual bool mouseMoved( const OIS::MouseEvent &arg );
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

  bool handleKeyPressed( OIS::KeyCode arg, int userID );
  bool handleKeyReleased( OIS::KeyCode arg, int userID );

  virtual void handleGameEnd();

  void handleLobbyState(void);
  void handleWaiting();

  bool startGame( const CEGUI::EventArgs& e );
  void loadLevel( const char* name );

  bool ExitToMenu( const CEGUI::EventArgs& e );
  bool togglePauseMenu( const CEGUI::EventArgs& e );
  void togglePauseMenu();

  Player* addPlayer(int userID, const char* name);
  void handleClientEvents(void);
  void updateClients(void);

  std::string currentLevelName;

  // User Input Variables
  btScalar MAX_TILT;
  float tiltDelay;
  CameraObject* mCameraObj;

  // Menu Variables
  CEGUI::Window *guiSheet, *scoreDisplay, *timeDisplay,
    *collectDisplay, *livesDisplay, *levelDisplay;

  CEGUI::Window *readyWindow, *goWindow;
  CEGUI::Window *pauseMenuSheet, *pauseQuit, *pauseReturn;

  CEGUI::Window *lobbySheet, *lobbySelectLevel, *lobbySelectCharacter,
    *lobbyLeave, *lobbyStart;

  CEGUI::Window *lobbyPlayerWindows[4];

  bool menuActive;
  bool ceguiActive;
  bool gameEnded;

  //Networking Stuff
  int myId;
  IPaddress ip, *remoteIP;
  bool inGame, chatFocus, waitingForClientsToLoad;

  // Game State Variables
  float timeLeft;  // In millis
  int countdown;
};
