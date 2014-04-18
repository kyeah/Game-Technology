
#include "SinglePlayerActivity.h"
#include "Interpolator.h"
#include "CameraObject.h"
#include <iostream>



float currTiltDelay = 0;
float totalTime = 500;

CameraObject::CameraObject(Ogre::Camera* cam){
	camera = cam;
	previousPosIsSet = false;
	doneFalling = false;
}

void CameraObject::update(Ogre::Vector3 newPosition, Ogre::Real elapsedTime){

  if(newPosition != previousPos ){

        Ogre::Vector3 direction = newPosition - previousPos;
        direction.normalise();

	if(direction[1] < 0 && !doneFalling) 
	{
		previousPos = newPosition;
		return;
	}
        else if(direction[1] == 0)
		doneFalling = true;

	Ogre::Vector3 newCamPos = newPosition - direction*fixedDist;
	btVector3 btVec1 = btVector3(newCamPos[0], newCamPos[1], newCamPos[2]);
	btVector3 camPos = btVector3(camera->getPosition()[0], camera->getPosition()[1], camera->getPosition()[2]);
	btVector3 btAvgPos = Interpolator::interpV3(currTiltDelay, elapsedTime, 500, btVec1, camPos);

	Ogre::Vector3 avgPos = Ogre::Vector3(btAvgPos[0], btAvgPos[1], btAvgPos[2]);
	
	if(avgPos.distance(newPosition) > fixedDist || avgPos.distance(newPosition) < fixedDist){
        	Ogre::Vector3 dir = newPosition - avgPos;
        	dir.normalise();
        	avgPos = newPosition - dir*fixedDist;
        }

	printf("setting position (%f,%f,%f)\n", avgPos[0], avgPos[1], avgPos[2]);
	camera->setPosition(avgPos[0], avgPos[1] > 0 ? avgPos[1] : -avgPos[1], avgPos[2]);
        camera->lookAt(newPosition);
        previousPos = newPosition;
  }
}

void CameraObject::setPreviousPosition(Ogre::Vector3 pos){
	previousPos = pos;
	previousPosIsSet = true;
}

void CameraObject::setFixedDistance(Ogre::Vector3 v1, Ogre::Vector3 v2){
	Ogre::Real dist = v1.distance(v2);
	cameraStartPosition = v2;
	fixedDist = dist;
}

