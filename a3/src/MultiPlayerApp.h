/*
-----------------------------------------------------------------------------
Filename:    MultiPlayerApp.h
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
#ifndef __MultiPlayerApp_h_
#define __MultiPlayerApp_h_

#include "common.h"
#include "BaseApplication.h"
#include "SDL_net.h"
#include "RacquetObject.h"
#include "Networking.h"

class MultiPlayerApp : public BaseApplication
{
public:
    MultiPlayerApp(bool _isHost);
    virtual ~MultiPlayerApp(void);
    bool frameStarted(const Ogre::FrameEvent &evt);


protected:
    virtual void createScene(void);
    void createCamera(void);
    void Connect();
    ServerPacket* Receive();
    void Send(char *msg, int len);
    void Close();

    void createFrameListener();
    bool keyPressed( const OIS::KeyEvent &arg );
    bool keyReleased(const OIS::KeyEvent &arg);

    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ); 

    Player* findPlayer(int userID);
    Player* addPlayer(int userID);

    IPaddress ip, *remoteIP;
    TCPsocket sd, csd;
    bool connected, isHost;

    int myId;
    Physics *mPhysics;
    Ball *mBall;

    
};

#endif // #ifndef __MultiPlayerApp_h_
