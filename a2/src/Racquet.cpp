#include "RacquetObject.h"

Racquet::Racquet(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName, Ogre::SceneNode* parentNode, 
                 Physics* _physics,
                 btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest, 
                 btVector3 _localInertia, btQuaternion *rotation) 
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, velocity, _mass, _rest, _localInertia, rotation)
{

  entity = mgr->createEntity(_entName, "sphere.mesh");
  entity->setCastShadows(true);
  entity->setMaterialName("Court/Racquet");
  
  node->attachObject(entity);
  node->scale(3,4,.2);

  node->_update(true,true);
  node->_updateBounds();
  Ogre::Vector3 s = node->_getWorldAABB().getHalfSize();
  collisionShape = new btBoxShape( btVector3(s[0],s[1],s[2]) );
  addToSimulator();

  body->setCollisionFlags(body->getCollisionFlags() |
                          btCollisionObject::CF_KINEMATIC_OBJECT);
  body->setActivationState(DISABLE_DEACTIVATION);
  
  body->setCcdMotionThreshold(1);
  body->setCcdSweptSphereRadius(0.4);
}

void Racquet::update(float elapsedTime) {
  if (physics->checkCollisions(this)) {
  }
}
