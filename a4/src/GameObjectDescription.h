#pragma once 
#include "GameObject.h"


class OgreBall : public GameObject{
 public:
  OgreBall(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String nodeName, Ogre::String meshName,
       Ogre::SceneNode* parentNode, Physics* physics,
       btVector3 origin=btVector3(0,0,0), btVector3 velocity=btVector3(0,0,0),
       btScalar mass=0.1f, btScalar rest=1.0f,
       btVector3 localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);

  void update(float elapsedTime);
};

class Plane : public GameObject {                                                                          
 public:                                                                                                   
  Plane(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, Ogre::String nodeName,     
        Ogre::SceneNode* parentNode, Physics* physics,                                                     
        btVector3 origin=btVector3(0,0,0), btVector3 velocity=btVector3(0,0,0),                            
        btScalar mass=0.0f, btScalar rest=0.9f,                                                            
        btVector3 localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);                               
                                                                                                           
  virtual void update(float elapsedTime);                                                                  
};
