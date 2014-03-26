/*
-----------------------------------------------------------------------------
Filename:    ClientApp.h
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
#ifndef __ClientApp_h_
#define __ClientApp_h_

#include "common.h"
#include "BaseApplication.h"
#include "BaseMultiplayerApp.h"
#include "SDL_net.h"
#include "RacquetObject.h"
#include "Networking.h"

class ClientApp : public BaseMultiplayerApp
{
public:
    ClientApp(void);
    ~ClientApp(void);
    bool frameStarted(const Ogre::FrameEvent &evt);

protected:
    void createCamera(void);
    void Connect();

    void Close();

    bool keyPressed( const OIS::KeyEvent &arg );
    bool keyReleased(const OIS::KeyEvent &arg);

    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ); 

    bool handleTextSubmitted( const CEGUI::EventArgs &e );
    virtual void createScene(void);

    int ids[4];
};

#endif // #ifndef __ClientApp_h_
