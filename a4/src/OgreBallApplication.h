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

#include "BaseApplication.h"

class OgreBallApplication : public BaseApplication
{
public:
    OgreBallApplication(void);
    virtual ~OgreBallApplication(void);

protected:
    virtual void createScene(void);
};

#endif // #ifndef __OgreBallApplication_h_
