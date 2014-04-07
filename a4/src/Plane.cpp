#include "GameObjectDescription.h"

Plane::Plane(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, Ogre::String _nodeName,
             Ogre::SceneNode* parentNode, Physics* _physics,
             btVector3 origin, btVector3 scale, btVector3 velocity, btScalar _mass, btScalar _rest,
             btVector3 _localInertia, btQuaternion *rotation)
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, scale, velocity, _mass, _rest, _localInertia, rotation)
{

  entity = mgr->createEntity(_entName, _meshName);
  entity->setCastShadows(true);

  node->attachObject(entity);

  Ogre::Vector3 s = entity->getBoundingBox().getHalfSize();
  collisionShape = new btBoxShape(btVector3(s[0], s[1], s[2]));
  addToSimulator();
}

void Plane::update(float elapsedTime) { 
  GameObject::update(elapsedTime);
}
