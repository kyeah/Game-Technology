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

#include "BaseApplication.h"
#include "SDL_net.h"
#include "RacquetObject.h"

class MultiPlayerApp : public BaseApplication
{
public:
    MultiPlayerApp(void);
    virtual ~MultiPlayerApp(void);
    bool frameStarted(const Ogre::FrameEvent &evt);


protected:
    virtual void createScene(void);
    void createCamera(void);
    void Connect();
    char* Receive();
    void Close();

    IPaddress ip, *remoteIP;
    TCPsocket sd, csd;
    bool connected;
    char buf[512];

    Physics *mPhysics;
    Racquet *mRacquet;
    Dude *mPlayer;
    Ball *mBall;


};

#endif // #ifndef __MultiPlayerApp_h_
