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

  bool toggleReady( const CEGUI::EventArgs &args );

  void handleLobbyState();
  virtual void handleGameEnd();

  void loadLevel( const char* name );
  bool waitForHosts();

  /*  bool menuActive, ceguiActive, gameEnded, levelLoaded,
      waitingScreenLoaded, readyToLoadLevel, inGame, chatFocus;*/
};
