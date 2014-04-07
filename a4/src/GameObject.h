#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include <btBulletDynamicsCommon.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>

#include "Collisions.h"
#include "OgreMotionState.h"
#include "Physics.h"

class GameObject {
 public:
  GameObject() {}

  GameObject(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName,
             Ogre::SceneNode* parentNode, Physics* _physics,
             btVector3 origin=btVector3(0,0,0),  btVector3 scale=btVector3(1,1,1), 
             btVector3 velocity=btVector3(0,0,0), btScalar _mass=0.0f,
             btScalar _rest=0.0f, btVector3 _localInertia=btVector3(0,0,0),  btQuaternion *rotation=0);

  void init();

  virtual void update(float elapsedTime);
  void updateTransform();
  void addToSimulator();
  
  void setKinematic(bool kinematic);
  void setColor(float ar, float ag, float ab,
                float dr, float dg, float db, float da,
                float sr, float sg, float sb, float sa);

  void setAmbient(float ar, float ag, float ab);
  void setDiffuse(float dr, float dg, float db, float da);
  void setSpecular(float sr, float sg, float sb, float sa);

  void setPosition(btVector3 position);
  void translate(btVector3 d);
  void setOrientation(btQuaternion quaternion);
  void rotate(btQuaternion q);
  void setVelocity(btVector3 velocity);

  void setInterpTimes(std::vector<float>& times);
  void setInterpPos(std::vector<btVector3>& positions);
  void setInterpRotTimes(std::vector<float>& times);
  void setInterpRot(std::vector<btQuaternion>& rotations);

  btVector3 getPosition();
  btQuaternion getOrientation();

  void setContactCallBack(BulletContactCallback *_callback) { cCallback = _callback; }

  std::vector<CollisionContext *> *getCollisionContexts() { return &contexts; }
  BulletContactCallback* getContactCallback() { return cCallback; }

  btRigidBody* getBody() { return body; }
  Ogre::SceneNode* getNode() { return node; }
  Ogre::Entity* getEntity() { return entity; }
  Ogre::String getEntityName() { return entName; }
  OgreMotionState* getMotionState() { return motionState; }

  void setSimID(int id) { simID = id; }
  int getSimID() { return simID; }

 protected:
  Ogre::String entName, nodeName;
  int simID;
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
  btVector3 initVel;

  std::vector<btVector3> posKnobs;
  std::vector<float> posInterpTimes;
  float totalInterpTime;

  std::vector<btQuaternion> rotKnobs;
  std::vector<float> rotInterpTimes;
  float totalInterpRotTime;

  float currentInterpPosTime;
  float currentInterpRotTime;
  
  bool needsUpdates;
  std::vector<CollisionContext *> contexts;
  BulletContactCallback* cCallback;
};

#endif

