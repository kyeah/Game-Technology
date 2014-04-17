#pragma once

#include "OgreBallApplication.h" 

class OgreBallApplication;

class Activity {
 public:
 Activity(OgreBallApplication *_app) : app(_app), score(0) {}
  
  virtual ~Activity(void) { }
  
  virtual void start(void) { }
  virtual bool frameStarted( Ogre::Real elapsedTime ) { }
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) { }
 
  virtual bool keyPressed( const OIS::KeyEvent &arg ) { }
  virtual bool keyReleased( const OIS::KeyEvent &arg ) { }

  virtual bool mouseMoved( const OIS::MouseEvent &arg ) { }
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) { }
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) { }

  virtual void handleGameEnd() { }
  OgreBallApplication *app;
  int score;
};
