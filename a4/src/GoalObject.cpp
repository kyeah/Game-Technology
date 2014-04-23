#include "GameObjectDescription.h"
#include "OgreBallApplication.h"
#include "Sounds.h"

Ogre::Entity* leftFlapEntity;
Ogre::Entity* rightFlapEntity;


GoalObject::GoalObject(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName,
             Ogre::SceneNode* parentNode, Physics* _physics,
             btVector3 origin, btVector3 scale, btVector3 velocity, btScalar _mass, btScalar _rest,
             btVector3 _localInertia, btQuaternion *rotation, Ogre::String hitSound)
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, scale, velocity, _mass, _rest, _localInertia, rotation)
{

  Ogre::SceneNode* leftFlap = node->createChildSceneNode("leftFlap", Ogre::Vector3(-125,60,0));
  Ogre::SceneNode* rightFlap = node->createChildSceneNode("rightFlap", Ogre::Vector3(125,60,0));
  Ogre::SceneNode* leftProtector = node->createChildSceneNode("leftProtector", Ogre::Vector3(-250,0,0));
  Ogre::SceneNode* rightProtector = node->createChildSceneNode("rightProtector", Ogre::Vector3(250,0,0));

  leftFlapEntity = mgr->createEntity(_entName + "leftFlap", "cube.mesh");
  leftFlapEntity->setMaterialName("OgreBall/NotPassed");
  rightFlapEntity = mgr->createEntity(_entName + "rightFlap", "cube.mesh");
  rightFlapEntity->setMaterialName("OgreBall/NotPassed");
  Ogre::Entity* leftProtectorEntity = mgr->createEntity(_entName + "leftProtector", "robot.mesh");
  Ogre::Entity* rightProtectorEntity = mgr->createEntity(_entName + "rightProtector", "robot.mesh");

  leftFlap->attachObject(leftFlapEntity);
  leftFlap->scale(2,1,0.01);
  rightFlap->attachObject(rightFlapEntity);
  rightFlap->scale(2,1,0.01);
  leftProtector->attachObject(leftProtectorEntity);
  leftProtector->scale(5,5,.01);
  rightProtector->attachObject(rightProtectorEntity);
  rightProtector->scale(5,5,.01);
  rightProtector->yaw((Ogre::Radian)3.14159);

  leftFlap->_update(true,true);
  rightFlap->_update(true,true);
  leftFlap->_updateBounds();
  rightFlap->_updateBounds();
  Ogre::Vector3 ls = leftFlap->_getWorldAABB().getHalfSize();
  Ogre::Vector3 rs = rightFlap->_getWorldAABB().getHalfSize();

  collisionShape = new btBoxShape(btVector3(ls[0] + rs[0], ls[1], ls[2]));
  addToSimulator(Collisions::CollisionTypes::COL_GOAL,
                 Collisions::goalColliders);

  if (rotation) rotate(*rotation);
  mHitSound = hitSound;
}

void GoalObject::update(float elapsedTime) {
  GameObject::update(elapsedTime);
  if(physics->checkCollisions(this)) {
    for (int i = 0; i < contexts.size(); i++) {
      if (contexts[i]->object && dynamic_cast<OgreBall*>(contexts[i]->object)) {
        Sounds::playSoundEffect(mHitSound.c_str(), (Sounds::MAX_VOLUME / 2));
	    leftFlapEntity->setMaterialName("OgreBall/Passed");
            rightFlapEntity->setMaterialName("OgreBall/Passed");
	OgreBallApplication::getSingleton()->activity->handleGameEnd();
      }
    }
  }
}
