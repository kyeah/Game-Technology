#pragma once

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include "Activity.h"
#include "GameObjectDescription.h"

class SinglePlayerActivity : public Activity {
 public:
  SinglePlayerActivity(OgreBallApplication *app, const char* levelName);
  virtual ~SinglePlayerActivity(void);

  void start(void);
  virtual bool frameStarted( Ogre::Real elapsedTime );
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

  virtual bool keyPressed( const OIS::KeyEvent &arg );
  virtual bool keyReleased( const OIS::KeyEvent &arg );

  virtual bool mouseMoved( const OIS::MouseEvent &arg );
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual void handleGameEnd();

  void loadLevel( const char* name );
  bool ExitToMenu( const CEGUI::EventArgs& e );
  bool togglePauseMenu( const CEGUI::EventArgs& e );
  void togglePauseMenu();

  const char* startingLevelName;

  btScalar MAX_TILT;
  btQuaternion lastTilt, currTilt, tiltDest;
  float currTiltDelay, tiltDelay;

  OgreBall *player;
  CEGUI::Window *guiSheet;
  bool menuActive;
  bool ceguiActive;
};
