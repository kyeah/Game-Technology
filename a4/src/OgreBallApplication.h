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

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>

#include "BaseApplication.h"
#include "GameObjectDescription.h"
#include "Physics.h"

class OgreBallApplication : public BaseApplication
{
 public:
  OgreBallApplication(void);
  virtual ~OgreBallApplication(void);

  bool frameStarted(const Ogre::FrameEvent &evt);

 protected:
  virtual void createScene(void);
  void createCamera(void);
  void createFrameListener(void);  // GUI Setup

  bool keyPressed( const OIS::KeyEvent &arg );
  bool keyReleased( const OIS::KeyEvent &arg );

  bool mouseMoved( const OIS::MouseEvent &arg );
  bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

  Ogre::Timer *mTimer;
  Physics *mPhysics;

  // CEGUI
  CEGUI::OgreRenderer* mRenderer;
  CEGUI::WindowManager* Wmgr;
  CEGUI::Window* menu;
};

#endif // #ifndef __OgreBallApplication_h_
