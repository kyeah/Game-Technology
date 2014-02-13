#include <OgreSubEntity.h>
#include "GameObject.h"
#include "OgreMotionState.h"

GameObject::GameObject(Ogre::String _entName, Ogre::String _nodeName, Ogre::SceneNode* parentNode, 
                       Physics* _physics,
                       btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest, 
                       btVector3 _localInertia, btQuaternion *rotation) 
  : entName(_entName), nodeName(_nodeName), 
    physics(_physics), mass(_mass), rest(_rest), inertia(_localInertia) {

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

void GameObject::addToSimulator(btVector3 position, btQuaternion *orientation) {
  //physics->addRigidBody(entity, node, collisionShape, mass, rest, inertia, position, orientation);
  
  //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
  updateTransform();
  motionState = new OgreMotionState(transform, node);
  
  //rigidbody is dynamic if and only if mass is non zero, otherwise static
  if (mass != 0.0f) collisionShape->calculateLocalInertia(mass, inertia);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, collisionShape, inertia);
  body = new btRigidBody(rbInfo);
  body->setRestitution(rest);
  body->setUserPointer(node);

  physics->addBody(body);
}

void GameObject::updateTransform() {
  Ogre::Vector3 pos = node->getPosition();
  transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
  Ogre::Quaternion qt = node->getOrientation();
  transform.setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
  if (motionState) motionState->setWorldTransform(transform);
  //if (motionState) motionState->updateWorldTransform(transform);
}
