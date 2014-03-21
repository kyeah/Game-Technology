#include "common.h"

const int DETAILS_HIGHSCORE = 0;
const int DETAILS_LASTSCORE = 2;
const int DETAILS_SCORE = 3;
const int DETAILS_GRAVITY = 4;

const int SWING_DELAY = 5;
const int UNSWING_DELAY = 10;

int gravMag = 7000;
bool pongMode = false;
bool right_mouse_button = false;

int highscore = 0;
int lastscore = 0;
int score = 0;

btVector3 racquetInitPos(0,700.0f,0);
btVector3 playerInitPos(100,-1200,-2245);
btVector3 *axis;

Ogre::Light* discolights[6];

BaseApplication *instance;

Player::Player(int _id) : id(_id) {}

Player* players[4];
