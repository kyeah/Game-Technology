
//#include <btBulletDynamicsCommon.h>
#include "GameObjectDescription.h"
#include "OgreBallApplication.h"
#include "Sounds.h"

Ogre::SceneNode* leftFlap;
Ogre::SceneNode* rightFlap;

Ogre::Entity* leftFlapEntity;
Ogre::Entity* rightFlapEntity;
int swinging = 0;
Ogre::Vector3 axis1;
Ogre::Vector3 axis2;

GoalObject::GoalObject(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName,
             Ogre::SceneNode* parentNode, Physics* _physics,
             btVector3 origin, btVector3 scale, btVector3 velocity, btScalar _mass, btScalar _rest,
             btVector3 _localInertia, btQuaternion *rotation, Ogre::String hitSound)
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, scale, velocity, _mass, _rest, _localInertia, rotation)
{

  Ogre::Vector3 nodeWorldPos = node->_getDerivedPosition();
   
  leftFlap = mgr->getRootSceneNode()->createChildSceneNode(_entName + "leftFlap");
  rightFlap = mgr->getRootSceneNode()->createChildSceneNode(_entName + "rightFlap");
  leftFlap->setPosition(Ogre::Vector3(nodeWorldPos[0] - 190, nodeWorldPos[1] + 60, nodeWorldPos[2]));
  rightFlap->setPosition(Ogre::Vector3(nodeWorldPos[0] + 190, nodeWorldPos[1] + 60, nodeWorldPos[2]));
  Ogre::SceneNode* leftChild = leftFlap->createChildSceneNode();
  Ogre::SceneNode* rightChild = rightFlap->createChildSceneNode();

/*
  leftFlap = node->createChildSceneNode("leftFlap", Ogre::Vector3(-125,60,0));
  rightFlap = node->createChildSceneNode("rightFlap", Ogre::Vector3(125,60,0));
*/
  Ogre::SceneNode* leftProtector = node->createChildSceneNode("leftProtector", Ogre::Vector3(-250,0,0));
  Ogre::SceneNode* rightProtector = node->createChildSceneNode("rightProtector", Ogre::Vector3(250,0,0));

  leftFlapEntity = mgr->createEntity(_entName + "leftFlap", "cube.mesh");
  leftFlapEntity->setMaterialName("OgreBall/NotPassed");
  rightFlapEntity = mgr->createEntity(_entName + "rightFlap", "cube.mesh");
  rightFlapEntity->setMaterialName("OgreBall/NotPassed");
  Ogre::Entity* leftProtectorEntity = mgr->createEntity(_entName + "leftProtector", "robot.mesh");
  Ogre::Entity* rightProtectorEntity = mgr->createEntity(_entName + "rightProtector", "robot.mesh");

  leftChild->attachObject(leftFlapEntity);
  leftChild->scale(2,1,0.01);
  leftChild->translate(65,0,0);
  rightChild->attachObject(rightFlapEntity);
  rightChild->scale(2,1,0.01);
  rightChild->translate(-65,0,0);
  leftProtector->attachObject(leftProtectorEntity);
  leftProtector->scale(5,5,.01);
  rightProtector->attachObject(rightProtectorEntity);
  rightProtector->scale(5,5,.01);
  rightProtector->yaw((Ogre::Radian)3.14159);

/*  Ogre::Vector3 leftFlapEdge = leftFlap->getPosition();
  axis1 = Ogre::Vector3(0, 5, 5);
  Ogre::Vector3 rightFlapEdge = rightFlap->getPosition();
  axis2 = Ogre::Vector3(5, 5, 0);
*/


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
 	swinging = 36;
	OgreBallApplication::getSingleton()->activity->handleGameEnd();
      }
    }
  }

  Ogre::Radian rad = (Ogre::Radian).01745328925;
  if(swinging > 0){
	if(swinging > 18){	
		leftFlap->rotate(Ogre::Quaternion(Ogre::Degree(10), Ogre::Vector3(0,1,0)), Ogre::Node::TransformSpace::TS_WORLD);
		rightFlap->rotate(Ogre::Quaternion(Ogre::Degree(-10), Ogre::Vector3(0,1,0)), Ogre::Node::TransformSpace::TS_WORLD);
	}else {
		leftFlap->rotate(Ogre::Quaternion(Ogre::Degree(-10), Ogre::Vector3(0,1,0)), Ogre::Node::TransformSpace::TS_WORLD);
		rightFlap->rotate(Ogre::Quaternion(Ogre::Degree(10), Ogre::Vector3(0,1,0)), Ogre::Node::TransformSpace::TS_WORLD);
	}
	swinging--;


  }

}
