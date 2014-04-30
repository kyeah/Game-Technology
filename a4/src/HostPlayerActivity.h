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
  virtual void handleGameEnd();

  void handleLobbyState(void);
  void loadLevel( const char* name );
  bool ExitToMenu( const CEGUI::EventArgs& e );
  bool togglePauseMenu( const CEGUI::EventArgs& e );
  void togglePauseMenu();
  Player* addPlayer(int userID);

  const char* currentLevelName;

  // User Input Variables
  btScalar MAX_TILT;
  btQuaternion lastTilt, currTilt, tiltDest;
  float currTiltDelay, tiltDelay;
  OgreBall* mOgreBall;  
  CameraObject* mCameraObj;

  // Menu Variables
  CEGUI::Window *guiSheet, *scoreDisplay, *timeDisplay, 
    *collectDisplay, *livesDisplay, *levelDisplay;

  bool menuActive;
  bool ceguiActive;
  bool gameEnded;
  
  //Networking Stuff
  int myId;
  IPaddress ip, *remoteIP;
  bool inGame;

  // Game State Variables
  OgreBall *player;
  float timeLeft;  // In millis
  int collectibles, lives;
};
