#include <OgreSubEntity.h>
#include "GameObject.h"

GameObject::GameObject(Ogre::String _entName, Ogre::String _nodeName, Ogre::SceneNode* parentNode, 
                       Physics* _physics,
                       btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest, 
                       btVector3 _localInertia, btQuaternion *rotation) 
  : entName(_entName), nodeName(_nodeName), 
    physics(_physics), mass(_mass), rest(_rest), inertia(_localInertia) {

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

btRigidBody* GameObject::addToSimulator(btVector3 position, btQuaternion *orientation) {
  physics->addRigidBody(entity, node, collisionShape, mass, rest, inertia, position, orientation);
}

void GameObject::updateTransform(btVector3 position, btQuaternion orientation) {
  
}
