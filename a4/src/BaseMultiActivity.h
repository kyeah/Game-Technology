#pragma once

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>

#include "Activity.h"
#include "CameraObject.h"
#include "GameObjectDescription.h"
#include "common.h"
#include "Networking.h"

class BaseMultiActivity : public Activity {
 public:
  BaseMultiActivity(OgreBallApplication *app);
  virtual ~BaseMultiActivity(void);
  virtual void close(void);

  virtual void start(void);
  virtual bool frameStarted( Ogre::Real elapsedTime );
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

  virtual bool keyPressed( const OIS::KeyEvent &arg );
  virtual bool keyReleased( const OIS::KeyEvent &arg );

  virtual bool mouseMoved( const OIS::MouseEvent &arg );
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

  virtual void handleCrossedFinishLine();
  virtual void handleLobbyState();
  virtual void handleGameEnd();

  virtual void loadLevel( const char* name );
  virtual bool ExitToMenu( const CEGUI::EventArgs& e );
  virtual bool SwitchToLobby( const CEGUI::EventArgs& e );
  virtual bool togglePauseMenu( const CEGUI::EventArgs& e );
  virtual void togglePauseMenu();

  virtual bool handleTextSubmitted( const CEGUI::EventArgs &e );

  Player* addPlayer(int userID, const char* name);

  void toggleChat();
  void togglePlayerReady(int userID);
  void addChatMessage(const char* msg);

  std::string currentLevelName;

  // Menu Variables
  CEGUI::Window *guiSheet, *scoreDisplay, *timeDisplay,
    *collectDisplay, *livesDisplay, *levelDisplay;

  CEGUI::Window *readyWindow, *goWindow;
  CEGUI::Window *pauseMenuSheet, *pauseQuit, *pauseReturn;

  CEGUI::Window *gameWonSheet, *gwGoal, *gwNextLevel, *gwBackToMenu,
    *gwViewLeaderboard, *gwTimeTaken, *gwCollectibles, *gwBonus, *gwScore, *gwHighscore,
    *gwNameEditText, *gwSubmitHighscore;

  CEGUI::Window *multiGameEndSheet, *multiGameEndFinish,
    *multiGameEndContinue;

  CEGUI::Window *lobbySheet, *lobbySelectLevel, *lobbySelectCharacter,
    *lobbyLeave, *lobbyStart, *lobbyNamebar;

  CEGUI::Window *lobbyPlayerWindows[4];

  CEGUI::Window *chatWindow;
  CEGUI::Listbox *chatbox;
  CEGUI::Editbox *chatEditbox;

  bool menuActive, ceguiActive, gameEnded, levelLoaded,
    waitingScreenLoaded, readyToLoadLevel, inGame, chatFocus;

  bool allowKeyPress, hideChatOnClose;

  int myId;
  int countdown;

  btScalar MAX_TILT;
  float tiltDelay;

  // Game State Variables
  OgreBall *player;
  float timeLeft;  // In millis
  int lives;
  
};
