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
	void setBall(OgreBall* o) {mOgreBall = o;}	
 	OgreBall* getBall(){return mOgreBall;}		

	TCPsocket csd;
        char name[128];
        btQuaternion lastTilt, currTilt, tiltDest;
        float currTiltDelay;
        int character;

private:
	int id;
	OgreBall *mOgreBall;
};

extern Player* players[4];
