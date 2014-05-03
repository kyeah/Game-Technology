#pragma once

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include "Activity.h"
#include "BaseMultiActivity.h"
#include "CameraObject.h"
#include "GameObjectDescription.h"
#include "common.h"
#include "Networking.h"

class ClientPlayerActivity : public BaseMultiActivity {
 public:
  ClientPlayerActivity(OgreBallApplication *app, ConnectAck *ack);
  virtual ~ClientPlayerActivity(void);
  virtual void close(void);

  virtual bool frameStarted( Ogre::Real elapsedTime );

  virtual bool keyPressed( const OIS::KeyEvent &arg );
  virtual bool keyReleased( const OIS::KeyEvent &arg );
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

  bool toggleReady( const CEGUI::EventArgs &args );
  bool handleTextSubmitted( const CEGUI::EventArgs &args );

  bool SwitchToPlayerSelectMenu( const CEGUI::EventArgs &args );
  void handlePlayerSelected(int i);

  void handleServerUpdates();
  virtual void handleCrossedFinishLine();

  void loadLevel( const char* name );
  bool waitForHosts();

  /*  bool menuActive, ceguiActive, gameEnded, levelLoaded,
      waitingScreenLoaded, readyToLoadLevel, inGame, chatFocus;*/
};
