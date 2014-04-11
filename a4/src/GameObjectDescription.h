#pragma once
#include "Collisions.h"
#include "GameObject.h"

class OgreBall : public GameObject{
 public:
  OgreBall(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String nodeName, Ogre::String meshName,
           Ogre::SceneNode* parentNode, Physics* physics,
           btVector3 origin=btVector3(0,0,0), btVector3 scale=btVector3(1,1,1),
           btVector3 velocity=btVector3(0,0,0),
           btScalar mass=0.1f, btScalar rest=1.0f,
           btVector3 localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);

  void update(float elapsedTime);
};

class Plane : public GameObject {
 public:
  Plane(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, Ogre::String nodeName,
        Ogre::SceneNode* parentNode, Physics* physics,
        btVector3 origin=btVector3(0,0,0), btVector3 scale=btVector3(1,1,1),
        btVector3 velocity=btVector3(0,0,0),
        btScalar mass=0.0f, btScalar rest=0.9f,
        btVector3 localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);

  virtual void update(float elapsedTime);
};

class Collectible : public GameObject {
  public:
    Collectible(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, Ogre::String _nodeName, Ogre::SceneNode* parentNode,
                   Physics* physics, 
                   btVector3 origin=btVector3(0,0,0), btVector3 scale=btVector3(1,1,1),
                   btVector3 velocity=btVector3(0,0,0), btScalar mass=0.0f, btScalar rest=.9f,
                   btVector3 localInertia=btVector3(0,0,0), btQuaternion *rotation=0);

    virtual void update(float elapsedTime);
};

class MeshObject : public GameObject {
 public:
  MeshObject(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, Ogre::String nodeName,
             Ogre::SceneNode* parentNode, Physics* physics,
             btVector3 origin=btVector3(0,0,0), btVector3 scale=btVector3(1,1,1),
             btVector3 velocity=btVector3(0,0,0),
             btScalar mass=0.0f, btScalar rest=0.7f,
             btVector3 localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);

  virtual void update(float elapsedTime);
};


class GoalObject : public GameObject {
 public:
  GoalObject(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String nodeName,
             Ogre::SceneNode* parentNode, Physics* physics,
             btVector3 origin=btVector3(0,0,0), btVector3 scale=btVector3(1,1,1),
             btVector3 velocity=btVector3(0,0,0),
             btScalar mass=0.0f, btScalar rest=0.1f,
             btVector3 localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);

  virtual void update(float elapsedTime);
};

class DecorativeObject : public GameObject {
  public:
    DecorativeObject(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, Ogre::String _nodeName, Ogre::SceneNode* parentNode,
                   Physics* physics, 
                   btVector3 origin=btVector3(0,0,0), btVector3 scale=btVector3(1,1,1),
                   btVector3 velocity=btVector3(0,0,0), btScalar mass=0.0f, btScalar rest=.9f,
                   btVector3 localInertia=btVector3(0,0,0), btQuaternion *rotation=0);

    virtual void update(float elapsedTime);
};
