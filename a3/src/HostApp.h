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
#include "Physics.h"
#include "RacquetObject.h"
#include "SDL_net.h"

extern int highscore;
extern int lastscore;
extern int score;

class HostApp : public BaseApplication
{
public:
    HostApp(void);
    virtual ~HostApp(void);

    Ogre::RenderWindow * getWindow(void) { return mWindow; }
    Ogre::Timer * getTimer(void) { return mTimer; }
    OIS::Mouse * getMouse(void) { return mMouse; }
    OIS::Keyboard * getKeyboard(void) { return mKeyboard; }

    bool frameStarted(const Ogre::FrameEvent &evt);
    void restart();

protected:
    virtual void createScene(void);
    void createCamera(void);
    void createFrameListener(void);
    void createRacquet(std::string, std::string, int, int, int);
    void createBall(int x, int y, int z, int vx, int vy, int vz);
    void createBall(std::string entName, std::string nodeName, int x, int y, int z, int vx, int vy, int vz);
    void createNewScoringPlane(int points = 2, btVector3 pos = btVector3(0,0,5000/2 - 20), 
                               btVector3 speed = btVector3(50,0,0),
                               btVector3 linearFactor = btVector3(1,0,0), 
                               btVector3 angularFactor = btVector3(0,0,0));

    bool keyPressed( const OIS::KeyEvent &arg );
    bool keyReleased(const OIS::KeyEvent &arg);

    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    bool handleKeyPressed(OIS::KeyCode arg, int userID);
    bool handleKeyReleased(OIS::KeyCode arg, int userID);

    bool handleMouseMoved( OIS::MouseState arg, int userID );
    bool handleMouseReleased( OIS::MouseState arg, OIS::MouseButtonID id, int userID );

    Player* findPlayer(int userID);
    Player* addPlayer(int userID);

    int worldWidth,worldLength,worldHeight;

/* server side stuff */ 
    void Connect();
    void Send(char *msg, int len);
    void Close();

    TCPsocket sd, csd; 
    IPaddress ip, *remoteIP;
    bool connected;
/* end */ 

    int myId;
    Ogre::Timer *mTimer;
    //    Ogre::Plane walls[4];
    Physics *mPhysics;
    Ball *mBall;

    btVector3 ballVelocity;
    btScalar MAX_SPEED;
};

//extern HostApp *sp_instance;

#endif // #ifndef __HostApp_h_



