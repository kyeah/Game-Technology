#include "common.h"

const int DETAILS_HIGHSCORE = 0;
const int DETAILS_LASTSCORE = 2;
const int DETAILS_SCORE = 3;
const int DETAILS_GRAVITY = 4;

const int DETAILS_TEAM1 = 5;
const int DETAILS_TEAM2 = 6;

const int SWING_DELAY = 5;
const int UNSWING_DELAY = 10;

const int MAX_PLAYERS = 4;

int gravMag = 7000;
bool pongMode = false;
bool right_mouse_button = false;

int highscore = 0;
int lastscore = 0;
int score = 0;
int team1Score = 0;
int team2Score = 0;

btVector3 racquetInitPos(0,700.0f,0);

btVector3 playerInitialPositions[] = {
  btVector3(400,-1200,-2245),
  btVector3(-400,-1200,2245),
  btVector3(-400,-1200,-2245),
  btVector3(400,-1200,2245)
};

btVector3 playerInitPos(100,-1200,-2245); // single player position
btVector3 *axis;

Ogre::Light* discolights[6];

BaseApplication *instance;

Player::Player(int _id) : id(_id) {
  mDirection = btVector3(0,0,0);
  oDirection = Ogre::Vector3(0,0,0);
  swing = 0;
  unswing = 0;
  movementSpeed = 1;
  axis = new btVector3(0,0,0);
  pongMode = false;
  right_mouse_button = false;
  csd = 0;
}

Player* players[4]; // Doesn't allow using constants in array initialization :(
