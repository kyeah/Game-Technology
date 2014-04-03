#pragma once 
#include "GameObject.h"


class OgreBall : public GameObject{
 public:
  OgreBall(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String nodeName,
       Ogre::SceneNode* parentNode, Physics* physics,
       btVector3 origin=btVector3(0,0,0), btVector3 velocity=btVector3(0,0,0),
       btScalar mass=0.1f, btScalar rest=1.0f,
       btVector3 localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);

  void update(float elapsedTime);
};



