#include <OgreSubEntity.h>
#include "Collisions.h"
#include "GameObject.h"
#include "OgreMotionState.h"

GameObject::GameObject(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName, Ogre::SceneNode* parentNode, 
                       Physics* _physics,
                       btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest, 
                       btVector3 _localInertia, btQuaternion *rotation) 
  : entName(_entName), nodeName(_nodeName), 
    physics(_physics), mass(_mass), rest(_rest), inertia(_localInertia), initVel(velocity) {

  if (!parentNode) {
    parentNode = mgr->getRootSceneNode();
  }
  
  node = parentNode->createChildSceneNode(_nodeName);
  
  node->setPosition(Ogre::Vector3(origin[0], origin[1], origin[2]));
  if (rotation)
    node->setOrientation(Ogre::Quaternion((*rotation)[0], (*rotation)[1], (*rotation)[2], (*rotation)[3]));
  
  transform.setIdentity();
  // Extend this class dude
}

void GameObject::setColor(float dr, float dg, float db, float da,
                          float sr, float sg, float sb, float sa) {
  Ogre::MaterialPtr mat = entity->getSubEntity(0)->getMaterial();
  Ogre::Pass *pass = mat->getTechnique(0)->getPass(0);
  pass->setDiffuse(dr, dg, db, da);
  pass->setSpecular(dr, dg, db, da);
  entity->setMaterialName(mat->getName());
}

void GameObject::addToSimulator() {
  //using motionState is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
  motionState = new OgreMotionState(node);
  
  //rigidbody is dynamic if and only if mass is non zero, otherwise static
  if (mass != 0.0f) collisionShape->calculateLocalInertia(mass, inertia);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, collisionShape, inertia);
  body = new btRigidBody(rbInfo);
  body->setRestitution(rest);
  body->setUserPointer(this);
  body->setLinearVelocity(initVel);
  physics->addObject(this);
  updateTransform();

  cCallback = new BulletContactCallback(*body, contexts);
}

void GameObject::updateTransform() {
  Ogre::Vector3 pos = node->getPosition();
  transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
  Ogre::Quaternion qt = node->getOrientation();
  transform.setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
  if (motionState) motionState->updateWorldTransform(transform);
}

void GameObject::setPosition(btVector3 position) {
  transform.setOrigin(position);
  motionState->setWorldTransform(transform);
  updateTransform();
}

void GameObject::translate(btVector3 d) {
  transform.setOrigin(transform.getOrigin() + d);
  motionState->setWorldTransform(transform);
}

void GameObject::setOrientation(btQuaternion quaternion) {
  transform.setRotation(quaternion);
  motionState->setWorldTransform(transform);
  updateTransform();
}

void GameObject::rotate(btQuaternion q) {
  transform.setRotation(transform.getRotation() + q);
  motionState->setWorldTransform(transform);
  updateTransform();
}
