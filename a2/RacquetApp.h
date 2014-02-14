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
#ifndef __RacquetApp_h_
#define __RacquetApp_h_

#include "BaseApplication.h"
#include "Physics.h"
#include "RacquetObject.h"

class RacquetApp : public BaseApplication
{
public:
    RacquetApp(void);
    virtual ~RacquetApp(void);

    Ogre::RenderWindow * getWindow(void) { return mWindow; }
    Ogre::Timer * getTimer(void) { return mTimer; }
    OIS::Mouse * getMouse(void) { return mMouse; }
    OIS::Keyboard * getKeyboard(void) { return mKeyboard; }

    bool frameStarted(const Ogre::FrameEvent &evt);

protected:
    virtual void createScene(void);
    void createCamera(void);
    void createRacquet(std::string, std::string, int, int, int);
    void createBall(int x, int y, int z, int vx, int vy, int vz);
    void createBall(std::string entName, std::string nodeName, int x, int y, int z, int vx, int vy, int vz);
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

    Ogre::Timer *mTimer;
    //    Ogre::Plane walls[4];
    Physics *mPhysics;
    Racquet *mRacquet;
};

#endif // #ifndef __RacquetApp_h_
