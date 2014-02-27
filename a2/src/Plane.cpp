#include "RacquetObject.h"

Plane::Plane(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, Ogre::String _nodeName, 
             Ogre::SceneNode* parentNode, Physics* _physics,
             btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest, 
             btVector3 _localInertia, btQuaternion *rotation) 
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, velocity, _mass, _rest, _localInertia, rotation)
{

  entity = mgr->createEntity(_entName, _meshName);
  entity->setCastShadows(true);
  
  node->attachObject(entity);

  Ogre::Vector3 s = entity->getBoundingBox().getHalfSize();
  collisionShape = new btBoxShape(btVector3(s[0], s[1], s[2]));
  addToSimulator();
  points = 0;
}

void Plane::update(float elapsedTime) {
  btVector3 velDir = body->getLinearVelocity();
  btScalar vel = velDir.length();
  int xdir = (velDir[0] > 0 ? 1 : -1);
  int ydir = (velDir[1] > 0 ? 1 : -1);
  int zdir = (velDir[2] > 0 ? 1 : -1);
  if (vel < initVel.length())
    body->setLinearVelocity(btVector3(initVel[0]*xdir, initVel[1]*ydir, initVel[2]*zdir));
}


ScoringPlane::ScoringPlane(int worldWidth, int worldLength, int worldHeight,
                           Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, 
                           Ogre::String _nodeName, 
                           Ogre::SceneNode* parentNode, Physics* _physics,
                           btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest, 
                           btVector3 _localInertia, btQuaternion *rotation) 
  : Plane(mgr, _entName, _meshName, _nodeName, parentNode, _physics, origin, velocity, _mass, _rest, _localInertia, rotation) { 

  width = worldWidth;
  length = worldLength;
  height = worldHeight;
  
  node->_update(true,true);
  node->_updateBounds();
  
  body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
  body->setActivationState(DISABLE_DEACTIVATION);
}

void ScoringPlane::update(float elapsedTime) {  
  btVector3 pos = getPosition();
  Ogre::Vector3 s = node->_getWorldAABB().getHalfSize();
  if (std::abs(pos[0]) + s[0] > length/2) initVel[0] *= -1;
  if (std::abs(pos[1]) + s[1] > height/2) initVel[1] *= -1;
  if (std::abs(pos[2]) + s[2] > width/2) initVel[2] *= -1;
  translate(initVel);
}
