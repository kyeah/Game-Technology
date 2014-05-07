#include "Collisions.h"

int Collisions::levelColliders = Collisions::COL_LEVEL | Collisions::COL_PLAYER;
int Collisions::playerColliders = Collisions::COL_LEVEL | Collisions::COL_PLAYER | Collisions::COL_COLLIDABLE;
int Collisions::collectibleColliders = Collisions::COL_PLAYER;
int Collisions::goalColliders = Collisions::COL_PLAYER;
int Collisions::collidableColliders = Collisions::COL_PLAYER;
