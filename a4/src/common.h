#pragma once

#include "GameObjectDescription.h"
#include <btBulletDynamicsCommon.h>
#include "BaseApplication.h"
#include "SDL_net.h"

extern const int MAX_PLAYERS;

class Player {
public:
	Player(int i);
	int getId() { return id; }
	void setBall(OgreBall* o) {mOgreBall = 0;}	
 	OgreBall* getBall(){return mOgreBall;}		

	TCPsocket csd;
private:
	int id;
	OgreBall *mOgreBall;
};

extern Player* players[4];
