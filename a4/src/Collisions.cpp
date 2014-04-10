#include "Collisions.h"

int Collisions::levelColliders = Collisions::COL_NOTHING;
int Collisions::playerColliders = Collisions::COL_LEVEL | Collisions::COL_PLAYER;
int Collisions::collectibleColliders = Collisions::COL_PLAYER;
int Collisions::goalColliders = Collisions::COL_PLAYER;
