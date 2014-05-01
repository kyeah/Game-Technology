#pragma once

#include "GameObjectDescription.h"
#include <btBulletDynamicsCommon.h>
#include "BaseApplication.h"
#include "CameraObject.h"
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
        int character;

        btQuaternion lastTilt, currTilt, tiltDest;
        float currTiltDelay;        

        Ogre::SceneNode *mCameraLookAtNode, *mCameraNode;        
        CameraObject *mCameraObj;

        bool ready;
        bool crossedFinishLine;

private:
	int id;
	OgreBall *mOgreBall;
};

extern Player* players[4];
