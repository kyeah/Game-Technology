#include "common.h"

const int MAX_PLAYERS = 4;

Player::Player(int _id) : id(_id) {
	csd = 0;
        crossedFinishLine = false;
        ready = false;
}

Player* players[4];
