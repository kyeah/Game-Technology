
#include "SinglePlayerActivity.h"
#include "Interpolator.h"
#include "CameraObject.h"
#include <iostream>



float currTiltDelay = 0;
float totalTime = 500;

CameraObject::CameraObject(Ogre::SceneNode* camLookAtNode, Ogre::SceneNode* camNode,
                           Ogre::Vector3 playerInitPos, Ogre::Vector3 camInitPos){
  cameraNode = camNode;
  lookAtNode = camLookAtNode;
  doneFalling = false;

  lookAtNode->_setDerivedPosition(playerInitPos);
  cameraNode->_setDerivedPosition(camInitPos);
  cameraNode->lookAt(playerInitPos + Ogre::Vector3(0,250,0), Ogre::SceneNode::TS_WORLD);
  setFixedDistance(playerInitPos, camInitPos);
}

void CameraObject::update(Ogre::Vector3 newPosition, Ogre::Real elapsedTime){

  lookAtNode->_setDerivedPosition(newPosition);
  lookAtNode->_setDerivedOrientation(Ogre::Quaternion());
  cameraNode->_setDerivedOrientation(Ogre::Quaternion());

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

    previousPos = newPosition;
    currTiltDelay = 0;
    destPos = newPosition - direction*fixedDist;
  

  btVector3 btVec1 = btVector3(destPos[0], destPos[1], destPos[2]);
  btVector3 camPos = btVector3(lastInterpPos[0], lastInterpPos[1], lastInterpPos[2]);
  btVector3 btAvgPos = Interpolator::interpV3(currTiltDelay, elapsedTime, 500, camPos, btVec1);

  Ogre::Vector3 avgPos = Ogre::Vector3(btAvgPos[0], btAvgPos[1], btAvgPos[2]);

  if(avgPos.distance(newPosition) > fixedDist || avgPos.distance(newPosition) < fixedDist){
    Ogre::Vector3 dir = newPosition - avgPos;
    dir.normalise();
    avgPos = newPosition - dir*fixedDist;
  }

  lastInterpPos = Ogre::Vector3(avgPos[0], std::max(newPosition[1] + 500, avgPos[1]), avgPos[2]);
  cameraNode->_setDerivedPosition(lastInterpPos);
  }
}


void CameraObject::setFixedDistance(Ogre::Vector3 v1, Ogre::Vector3 v2){
  Ogre::Real dist = v1.distance(v2);
  previousPos = lastInterpPos = destPos = v2;
  cameraNode->setPosition(v2);
  fixedDist = dist;
}
