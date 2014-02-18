#include "RacquetObject.h"

Ball::Ball(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName, Ogre::SceneNode* parentNode, 
           Physics* _physics,
           btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest, 
           btVector3 _localInertia, btQuaternion *rotation) 
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, velocity, _mass, _rest, _localInertia, rotation)
{
  
  entity = mgr->createEntity(_entName, "sphere.mesh");
  entity->setCastShadows(true);
  
  node->attachObject(entity);
  
  // Change Entity Color
  //  setColor(0,1,0,0.1,  1,1,1,0.4);

  Ogre::Vector3 s = entity->getBoundingBox().getHalfSize();
  collisionShape = new btSphereShape(s[0]);
  addToSimulator();
}

void Ball::update(float elapsedTime) {
  if (physics->checkCollisions(this)) {
    std::cout << "BALL COLLISION" << std::endl;
    translate(btVector3(1000,1000,1000));
  }
}
