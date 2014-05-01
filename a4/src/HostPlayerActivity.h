#pragma once

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include "BaseMultiActivity.h"
#include "Activity.h"
#include "CameraObject.h"
#include "GameObjectDescription.h"
#include "Networking.h"
#include "common.h"

class HostPlayerActivity : public BaseMultiActivity {
 public:
  HostPlayerActivity(OgreBallApplication *app, const char* levelName);
  virtual ~HostPlayerActivity(void);
  virtual void close(void);

  void start(void);
  virtual bool frameStarted( Ogre::Real elapsedTime );

  virtual bool keyPressed( const OIS::KeyEvent &arg );
  virtual bool keyReleased( const OIS::KeyEvent &arg );

  bool handleKeyPressed( OIS::KeyCode arg, int userID );
  bool handleKeyReleased( OIS::KeyCode arg, int userID );

  void handleCrossedFinishLine( int id );
  virtual void handleGameEnd();

  void handleLobbyState(void);
  void handleWaiting();

  bool startGame( const CEGUI::EventArgs& e );
  void loadLevel( const char* name );

  void handleClientEvents(void);
  void updateClients(void);

  //Networking Stuff
  IPaddress ip, *remoteIP;
  bool waitingForClientsToLoad;
};
