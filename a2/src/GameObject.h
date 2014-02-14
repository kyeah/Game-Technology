#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include <btBulletDynamicsCommon.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>

#include "OgreMotionState.h"
#include "Physics.h"

class GameObject {
 public:
  GameObject() {}

  GameObject(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String nodeName, Ogre::SceneNode* parentNode, 
             Physics* physics,
             btVector3 origin=btVector3(0,0,0), btVector3 velocity=btVector3(0,0,0), btScalar mass=0.0f, 
             btScalar rest=0.0f, btVector3 localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);

  void updateTransform();
  void addToSimulator();

  void setColor(float dr, float dg, float db, float da,
                float sr, float sg, float sb, float sa);

  btRigidBody* getBody() { return body; }
  void setPosition(btVector3 position);
  void translate(btVector3 d);
  void setOrientation(btQuaternion quaternion);
  void rotate(btQuaternion q);

 protected:
  Ogre::String entName, nodeName;
  Physics *physics;
  Ogre::Entity *entity;
  Ogre::SceneNode *node;
  OgreMotionState *motionState;
  btCollisionShape *collisionShape;
  btScalar mass;
  btScalar rest;
  btRigidBody *body;
  btTransform transform;
  btVector3 inertia;
};

#endif
