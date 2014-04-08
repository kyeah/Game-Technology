#pragma once

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include "Activity.h"

class SinglePlayerActivity : public Activity {
 public:
 SinglePlayerActivity(OgreBallApplication *app) : Activity(app) {}
  virtual ~SinglePlayerActivity(void);
  
  virtual void start(void);
  virtual bool frameStarted( const Ogre::FrameEvent &evt );
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
  
  virtual bool keyPressed( const OIS::KeyEvent &arg );
  virtual bool keyReleased( const OIS::KeyEvent &arg );
  
  virtual bool mouseMoved( const OIS::MouseEvent &arg );
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

  void loadLevel( char* name );
};
