#pragma once
#include "GameObject.h"

class Ball : public GameObject {
 public:
  Ball(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String nodeName, 
       Ogre::SceneNode* parentNode, Physics* physics,
       btVector3 origin=btVector3(0,0,0), btVector3 velocity=btVector3(0,0,0), 
       btScalar mass=0.1f, btScalar rest=1.0f, 
       btVector3 localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);

  static void *changeWall(void *entity);
  void update(float elapsedTime);
  bool bouncedOnce;
};

class Racquet : public GameObject {
 public:  
  Racquet(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String nodeName, 
          Ogre::SceneNode* parentNode, Physics* physics,
          btVector3 origin=btVector3(0,0,0), btVector3 velocity=btVector3(0,0,0), 
          btScalar mass=0.0f, btScalar rest=0.95f, 
          btVector3 localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);

  void update(float elapsedTime);
};

class Dude : public GameObject{
 public:
  Dude(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String nodeName,
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
  Ogre::Entity* getEntity() { return entity; }
  int points;
};

class ScoringPlane : public Plane {
 public:  
  ScoringPlane(int worldWidth, int worldLength, int worldHeight,
               Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, Ogre::String nodeName, 
               Ogre::SceneNode* parentNode, Physics* physics,
               btVector3 origin=btVector3(0,0,0), btVector3 velocity=btVector3(0,0,0), 
               btScalar mass=0.0f, btScalar rest=0.9f, 
               btVector3 localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);

  void update(float elapsedTime);
  void cycleColor();
  int width, length, height;
};
