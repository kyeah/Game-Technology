/*
-----------------------------------------------------------------------------
Filename:    BallApp.h
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
#ifndef __HostApp_h_
#define __HostApp_h_

#include "common.h"
#include "BaseApplication.h"
#include "BaseMultiplayerApp.h"
#include "Physics.h"
#include "RacquetObject.h"
#include "SDL_net.h"

class HostApp : public BaseMultiplayerApp
{
public:
    HostApp(void);
    ~HostApp(void);
    bool frameStarted(const Ogre::FrameEvent &evt);

protected:
    void createCamera(void);
    
    bool keyPressed( const OIS::KeyEvent &arg );
    bool keyReleased(const OIS::KeyEvent &arg);

    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    bool handleKeyPressed(OIS::KeyCode arg, int userID);
    bool handleKeyReleased(OIS::KeyCode arg, int userID);

    bool handleMouseMoved( OIS::MouseState arg, int userID );
    bool handleMouseReleased( OIS::MouseState arg, OIS::MouseButtonID id, int userID );

    bool handleTextSubmitted( const CEGUI::EventArgs &e );
    virtual void createScene(void);

/* server side stuff */ 
    void Connect();
    void Close();

    TCPsocket sd, csd; 
    IPaddress ip, *remoteIP;
/* end */ 
};

#endif 



