#ifndef _CAMERAOBJECT_H
#define _CAMERAOBJECT_H

#include "Interpolator.h"

using namespace std;

class CameraObject{
 public:
  CameraObject(Ogre::Camera* cam);
  void update(Ogre::Vector3 newPosition, Ogre::Real elapsedTime);
  void setPreviousPosition(Ogre::Vector3 pos);
  void setFixedDistance(Ogre::Vector3 v1, Ogre::Vector3 v2);

  Ogre::Camera *camera;
  Ogre::Vector3 previousPos;
  Ogre::Vector3 destPos;
  Ogre::Real fixedDist;
  Ogre::Vector3 cameraStartPosition;
  bool doneFalling;
  bool previousPosIsSet;

  float totalTime;
  float elapsedTime;
  float currTiltDelay;
};

#endif
