
//#include <btBulletDynamicsCommon.h>
#include "GameObjectDescription.h"
#include "OgreBallApplication.h"
#include "Sounds.h"

Ogre::SceneNode* leftFlap;
Ogre::SceneNode* rightFlap;
Ogre::SceneNode* tempChildL;
Ogre::SceneNode* tempChildR;

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
  tempChildL = node->createChildSceneNode("blehL", Ogre::Vector3(-190, 60, 0));
  tempChildR = node->createChildSceneNode("blehR", Ogre::Vector3(190, 60, 0));
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
  
  Ogre::Vector3 leftPos = tempChildL->_getDerivedPosition();
  Ogre::Vector3 rightPos = tempChildR->_getDerivedPosition();
  leftFlap->setPosition(Ogre::Vector3(leftPos[0], leftPos[1], leftPos[2]));
  rightFlap->setPosition(Ogre::Vector3(rightPos[0], rightPos[1], rightPos[2]));

// a bunch of shit for fixing the leap level stuff. i don even known manz

/*  Ogre::Vector3 LedgePos = leftProtector->_getDerivedPosition();
  Ogre::Vector3 RedgePos = rightProtector->_getDerivedPosition(); 
  Ogre::Vector3 dirReal = LedgePos - RedgePos;
  dirReal.normalise();
  printf("direction between robots (%f,%f,%f)\n", dirReal[0], dirReal[1], dirReal[2]);
  
  Ogre::AxisAlignedBox bb = leftFlap->_getWorldAABB();
//  const Ogre::Vector3* corners = bb.getAllCorners();
  Ogre::Vector3 dirFake = bb.getCorner(Ogre::AxisAlignedBox::NEAR_LEFT_TOP) - bb.getCorner(Ogre::AxisAlignedBox::NEAR_RIGHT_TOP);
//  Ogre::Vector3 leftFlapPos = leftFlap->_getDerivedPosition();
//Ogre::Vector3 leftChildPos = leftChild->_getDerivedPosition();
//  Ogre::Vector3 dirFake = leftFlapPos - leftChildPos;
  dirFake.normalise();
  printf("direction between panels (%f,%f,%f)\n", dirFake[0], dirFake[1], dirFake[2]);
  Ogre::Radian angle = dirReal.angleBetween(dirFake);
  printf("angle is %f\n", angle);
  leftFlap->yaw(angle);
  rightFlap->yaw(angle);
*/
  leftChild->_update(true,true);
  rightChild->_update(true,true);
  leftChild->_updateBounds();
  rightChild->_updateBounds();
  Ogre::Vector3 ls = leftChild->_getWorldAABB().getHalfSize();
  Ogre::Vector3 rs = rightChild->_getWorldAABB().getHalfSize();

  collisionShape = new btBoxShape(btVector3(ls[0] + rs[0], ls[1], ls[2]));
  addToSimulator(Collisions::CollisionTypes::COL_GOAL,
                 Collisions::goalColliders);

  if (rotation) rotate(*rotation);
  mHitSound = hitSound;
}

void GoalObject::update(float elapsedTime) {

  Ogre::Vector3 leftPos = tempChildL->_getDerivedPosition();
  Ogre::Vector3 rightPos = tempChildR->_getDerivedPosition();
  leftFlap->setPosition(Ogre::Vector3(leftPos[0], leftPos[1], leftPos[2]));
  rightFlap->setPosition(Ogre::Vector3(rightPos[0], rightPos[1], rightPos[2]));
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
