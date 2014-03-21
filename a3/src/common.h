#pragma once

#include <btBulletDynamicsCommon.h>
#include "BaseApplication.h"

extern const int DETAILS_HIGHSCORE; 
extern const int DETAILS_LASTSCORE;
extern const int DETAILS_SCORE;
extern const int DETAILS_GRAVITY;
                                                                                                       
extern const int SWING_DELAY;
extern const int UNSWING_DELAY;

extern int gravMag;
extern bool pongMode;
extern bool right_mouse_button;

extern int highscore;
extern int lastscore;
extern int score;

extern btVector3 racquetInitPos; 
extern btVector3 playerInitPos;  
extern btVector3 *axis; // Swing rotation axis                                                                

extern Ogre::Light* discolights[6];
extern BaseApplication *instance;
