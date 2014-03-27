#pragma once

#include <btBulletDynamicsCommon.h>
#include "BaseApplication.h"
#include "RacquetObject.h"
#include "SDL_net.h"

extern const int DETAILS_HIGHSCORE; 
extern const int DETAILS_LASTSCORE;
extern const int DETAILS_SCORE;
extern const int DETAILS_GRAVITY;

extern const int DETAILS_TEAM1;
extern const int DETAILS_TEAM2;
                                                                                                       
extern const int SWING_DELAY;
extern const int UNSWING_DELAY;

extern const int MAX_PLAYERS;

extern int gravMag;
extern bool pongMode;
extern bool right_mouse_button;

extern int highscore;
extern int lastscore;
extern int score;
extern int team1Score;
extern int team2Score;

extern btVector3 racquetInitPos; 
extern btVector3 playerInitPos;  
extern btVector3 playerInitialPositions[];
extern btVector3 *axis; // Swing rotation axis                                                                

extern Ogre::Light* discolights[6];
extern BaseApplication *instance;

class Player {
 public:
  Player(int id);
  
  int getId() { return id; }
  Dude* getNode() { return mNode; }
  Racquet* getRacquet() { return mRacquet; }
  void setNode(Dude *d) { mNode = d; }
  void setRacquet(Racquet *r) { mRacquet = r; }

  TCPsocket csd;
  btVector3 mDirection;
  Ogre::Vector3 oDirection;
  float swing, unswing;
  btVector3 *axis;  // Swing axis
  float movementSpeed;
  bool pongMode;
  bool right_mouse_button;
  
 private:
  int id;
  Dude *mNode;
  Racquet *mRacquet;
};

extern Player* players[4];
