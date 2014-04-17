/*
  -----------------------------------------------------------------------------
  Filename:    OgreBallApplication.h
  -----------------------------------------------------------------------------

  This source file is part of the
  ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
  //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
  / \_// (_| | | |  __/  \  /\  /| |   <| |
  \___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
  |___/
  Tutorial Framework
  http://www.ogre3d.org/tikiwiki/
  -----------------------------------------------------------------------------
*/
#ifndef __OgreBallApplication_h_
#define __OgreBallApplication_h_

#include "BaseApplication.h"
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>

#include "Activity.h"
#include "BaseApplication.h"
#include "GameObjectDescription.h"
#include "LevelLoader.h"
#include "Physics.h"
#include "../libs/ConfigLoader.hpp"

class Activity;

class OgreBallApplication : public BaseApplication
{
 public:
  static OgreBallApplication *instance;
  static CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID);

  static OgreBallApplication* getSingleton() { return instance; }

  OgreBallApplication(void);
  virtual ~OgreBallApplication(void);

  void destroyAllEntitiesAndNodes(void);
  void switchActivity(Activity *activity);

  bool frameStarted( const Ogre::FrameEvent &evt );
  bool frameRenderingQueued( const Ogre::FrameEvent& evt );

 protected:
  virtual void createScene(void);
  void createCamera(void);
  void loadResources(void);  // GUI Setup

  bool keyPressed( const OIS::KeyEvent &arg );
  bool keyReleased( const OIS::KeyEvent &arg );

  bool mouseMoved( const OIS::MouseEvent &arg );
  bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

 public:
  Activity *activity;  // Our application will dispatch events to activities to deal with different game states.

  Ogre::Timer *mTimer;
  bool paused;

  Physics *mPhysics;
  LevelLoader *levelLoader;
  Ogre::SceneNode *levelRoot;

  // CEGUI
  CEGUI::OgreRenderer* mRenderer;
  CEGUI::WindowManager* Wmgr;
  CEGUI::Window* menu;
  
  OgreBall *mPlayer;
  btVector3 playerVelocity;
  btScalar MAX_SPEED;
  CEGUI::Window *sheet;
  
};

#endif // #ifndef __OgreBallApplication_h_
