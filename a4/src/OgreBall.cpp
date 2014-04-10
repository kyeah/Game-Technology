#include "OgreBallApplication.h"
#include "GameObject.h"
#include "GameObjectDescription.h"

OgreBall::OgreBall(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName, Ogre::String _meshName, Ogre::SceneNode* parentNode,
                   Physics* _physics,
                   btVector3 origin, btVector3 scale,
                   btVector3 velocity, btScalar _mass, btScalar _rest,
                   btVector3 _localInertia, btQuaternion *rotation)
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, scale, velocity, _mass, _rest, _localInertia, rotation)
{

  entity = mgr->createEntity(_entName, "sphere.mesh");
  entity->setCastShadows(true);

  entity->setMaterialName("OgreBall/Transparent");

  node->attachObject(entity);

  Ogre::Entity* charHead = mgr->createEntity(_entName + "head", _meshName);
  Ogre::SceneNode* headNode = node->createChildSceneNode(_nodeName + "head");
  headNode->attachObject(charHead);
  headNode->scale(2,2,2);

  node->_update(true,true);
  node->_updateBounds();
  Ogre::Vector3 s = node->_getWorldAABB().getHalfSize();
  collisionShape = new btSphereShape(s[0]);
  addToSimulator(Collisions::CollisionTypes::COL_PLAYER,
                 Collisions::playerColliders);

  // body->setCcdMotionThreshold(1);
  // body->setCcdSweptSphereRadius(0.4);

  setAmbient(0.5,0.5,0.9);
  setSpecular(0.1,0,0,0.4);
  if (rotation) rotate(*rotation);
}

void OgreBall::update(float elapsedTime) {
}
