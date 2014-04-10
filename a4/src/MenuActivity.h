#pragma once

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include "Activity.h"

class MenuActivity : public Activity {
 public:
 MenuActivity(OgreBallApplication *app) : Activity(app) {}
  
  virtual void start(void);
  virtual bool frameStarted( Ogre::Real elapsedTime );
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
  
  virtual bool keyPressed( const OIS::KeyEvent &arg );
  virtual bool keyReleased( const OIS::KeyEvent &arg );
  
  virtual bool mouseMoved( const OIS::MouseEvent &arg );
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

  bool SwitchToMainMenu(const CEGUI::EventArgs& e);
  bool SwitchToLevelSelectMenu(const CEGUI::EventArgs& e);
  bool SwitchToMultiMenu(const CEGUI::EventArgs& e);

  bool StartSinglePlayer(const CEGUI::EventArgs& e);

  bool quit(const CEGUI::EventArgs& e);
};
