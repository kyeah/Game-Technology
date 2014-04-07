#include <OgreSubEntity.h>
#include "Collisions.h"
#include "GameObject.h"
#include "OgreMotionState.h"

GameObject::GameObject(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName, Ogre::SceneNode* parentNode,
                       Physics* _physics,
                       btVector3 origin, btVector3 scale,
                       btVector3 velocity, btScalar _mass, btScalar _rest,
                       btVector3 _localInertia, btQuaternion *rotation)
  : entName(_entName), nodeName(_nodeName),
    physics(_physics), mass(_mass), rest(_rest), inertia(_localInertia), initVel(velocity) {

  if (!parentNode) {
    parentNode = mgr->getRootSceneNode();
  }

  node = parentNode->createChildSceneNode(_nodeName);

  node->translate(Ogre::Vector3(origin[0], origin[1], origin[2]));
  if (rotation)
    node->rotate(Ogre::Quaternion((*rotation)[0], (*rotation)[1], (*rotation)[2], (*rotation)[3]));

  node->scale(scale[0], scale[1], scale[2]);
  transform.setIdentity();

  currentInterpPosTime = 0;
  currentInterpRotTime = 0;
  // Extend this class dude
}

void GameObject::setKinematic(bool kinematic) {
  if (kinematic && body) {
    body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    body->setActivationState(DISABLE_DEACTIVATION);
    body->setMassProps(0, btVector3(0,0,0));
  } else if (body) {
    body->setCollisionFlags(body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
    body->activate(true);
  }
}

void GameObject::setInterpTimes(std::vector<float>& times) {
  totalInterpTime = 0;
  for (int i = 0; i< times.size(); i++) {
    float t = times[i];
    times[i] += totalInterpTime;
    totalInterpTime += t;
  }

  posInterpTimes = times;
}

void GameObject::setInterpPos(std::vector<btVector3>& positions) {
  posKnobs = positions;
}

void GameObject::setInterpRotTimes(std::vector<float>& times) {
  totalInterpRotTime = 0;
  for (int i = 0; i< times.size(); i++) {
    float t = times[i];
    times[i] += totalInterpRotTime;
    totalInterpRotTime += t;
  }

  rotInterpTimes = times;
}

void GameObject::setInterpRot(std::vector<btQuaternion>& rotations) {
  rotKnobs = rotations;
}

void GameObject::setColor(float ar, float ag, float ab,
                          float dr, float dg, float db, float da,
                          float sr, float sg, float sb, float sa) {

  Ogre::MaterialPtr mat = entity->getSubEntity(0)->getMaterial();
  mat = mat->clone(mat->getName() + entName + "1");
  Ogre::Pass *pass = mat->getTechnique(0)->getPass(0);
  pass->setAmbient(ar,ag,ab);
  pass->setDiffuse(dr, dg, db, da);
  pass->setSpecular(sr, sg, sb, sa);
  entity->setMaterialName(mat->getName());
}

void GameObject::setAmbient(float ar, float ag, float ab) {
  Ogre::MaterialPtr mat = entity->getSubEntity(0)->getMaterial();
  mat = mat->clone(mat->getName() + entName + "1");
  Ogre::Pass *pass = mat->getTechnique(0)->getPass(0);
  pass->setAmbient(ar,ag,ab);
  entity->setMaterialName(mat->getName());
}

void GameObject::setDiffuse(float dr, float dg, float db, float da) {
  Ogre::MaterialPtr mat = entity->getSubEntity(0)->getMaterial();
  mat = mat->clone(mat->getName() + entName + "1");
  Ogre::Pass *pass = mat->getTechnique(0)->getPass(0);
  pass->setDiffuse(dr, dg, db, da);
  entity->setMaterialName(mat->getName());
}

void GameObject::setSpecular(float sr, float sg, float sb, float sa) {
  Ogre::MaterialPtr mat = entity->getSubEntity(0)->getMaterial();
  mat = mat->clone(mat->getName() + entName + "1");
  Ogre::Pass *pass = mat->getTechnique(0)->getPass(0);
  pass->setSpecular(sr, sg, sb, sa);
  entity->setMaterialName(mat->getName());
}
void GameObject::addToSimulator() {
  motionState = 0;
  updateTransform();
  //using motionState is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
  motionState = new OgreMotionState(transform, node);

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
  Ogre::Vector3 pos = node->_getDerivedPosition();
  transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
  Ogre::Quaternion qt = node->_getDerivedOrientation();
  transform.setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
  if (motionState) motionState->updateWorldTransform(transform);
}

void GameObject::setPosition(btVector3 position) {
  motionState->getWorldTransform(transform);
  transform.setOrigin(position);
  motionState->setWorldTransform(transform);
}

void GameObject::translate(btVector3 d) {
  motionState->getWorldTransform(transform);
  transform.setOrigin(transform.getOrigin() + d);
  motionState->setWorldTransform(transform);
  body->setWorldTransform(transform);
}

void GameObject::setOrientation(btQuaternion quaternion) {
  motionState->getWorldTransform(transform);
  transform.setRotation(quaternion);
  motionState->setWorldTransform(transform);
}
void GameObject::rotate(btQuaternion q) {
  motionState->getWorldTransform(transform);
  transform.setRotation(transform.getRotation()*q);
  motionState->setWorldTransform(transform);
}

btVector3 GameObject::getPosition() {
  motionState->getWorldTransform(transform);
  return transform.getOrigin();
}

btQuaternion GameObject::getOrientation() {
  motionState->getWorldTransform(transform);
  return transform.getRotation();
}

void GameObject::setVelocity(btVector3 vel) {
  initVel = vel;
  body->setLinearVelocity(initVel);
}

void GameObject::update(float elapsedTime) {
  if (posKnobs.size() > 0) {
    currentInterpPosTime = fmod((currentInterpPosTime + elapsedTime), totalInterpTime);

    for (int i = 1; i < posInterpTimes.size(); i++) {
      if (currentInterpPosTime < posInterpTimes[i]) {
        // Interp time is between position (i-1) and position i
        btVector3 first = posKnobs[i-1];
        btVector3 second = posKnobs[i];

        float dt = currentInterpPosTime - posInterpTimes[i-1];
        float proportion = dt/(posInterpTimes[i]-posInterpTimes[i-1]);

        btVector3 pos = first.lerp(second, proportion);
        setPosition(pos);
        break;
      }
    }
  }

  if (rotKnobs.size() > 0) {
    currentInterpRotTime = fmod((currentInterpRotTime + elapsedTime), totalInterpRotTime);

    for (int i = 1; i < rotInterpTimes.size(); i++) {
      if (currentInterpRotTime < rotInterpTimes[i]) {
        btQuaternion first = rotKnobs[i-1];
        btQuaternion second = rotKnobs[i];

        float dt = currentInterpRotTime - rotInterpTimes[i-1];
        float proportion = dt/(rotInterpTimes[i] - rotInterpTimes[i-1]);
        
        btQuaternion rot = first.slerp(second, proportion);
        setOrientation(rot);
        break;
      }
    }
  }
}
