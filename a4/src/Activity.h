#pragma once
#include "OgreBallApplication.h" 

class OgreBallApplication;

class Activity {
 public:
 Activity(OgreBallApplication *_app) : app(_app) {}
  
  virtual void start(void) = 0;
  virtual bool frameStarted( Ogre::Real elapsedTime ) = 0;
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt) = 0;
 
  virtual bool keyPressed( const OIS::KeyEvent &arg ) = 0;
  virtual bool keyReleased( const OIS::KeyEvent &arg ) = 0;

  virtual bool mouseMoved( const OIS::MouseEvent &arg ) = 0;
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) = 0;
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) = 0;

  OgreBallApplication *app;
};
