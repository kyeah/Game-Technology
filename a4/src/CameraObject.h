#ifndef _CAMERAOBJECT_H
#define _CAMERAOBJECT_H

#include "Interpolator.h"

using namespace std;

class CameraObject{
 public:
  CameraObject(Ogre::SceneNode* lookAtNode, Ogre::SceneNode* camNode, 
               Ogre::Vector3 playerInitPos, Ogre::Vector3 camInitPos);

  void update(Ogre::Vector3 newPosition, Ogre::Real elapsedTime);
  void setFixedDistance(Ogre::Vector3 v1, Ogre::Vector3 v2);

  Ogre::SceneNode *cameraNode, *lookAtNode;
  Ogre::Vector3 previousPos;
  Ogre::Vector3 destPos;
  Ogre::Vector3 lastInterpPos;
  Ogre::Real fixedDist;
  bool doneFalling;

  float totalTime;
  float elapsedTime;
  float currTiltDelay;
};

#endif
