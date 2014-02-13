#include "RacquetObject.h"

Ball::Ball(Ogre::SceneManager *mgr, Ogre::SceneNode* parentNode, Physics* _physics,
           btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest, 
           btVector3 _localInertia, btQuaternion *rotation) {
  
  static int ballID;
  
  std::stringstream ss;
  ss << "myBall" << ballID;
  std::string ent = ss.str();
  ss << "node";
  ballID++;
  
  Ball(mgr, ent, ss.str(), parentNode, _physics, origin, velocity, _mass, _rest, _localInertia, rotation);

}

Ball::Ball(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName, Ogre::SceneNode* parentNode, 
           Physics* _physics,
           btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest, 
           btVector3 _localInertia, btQuaternion *rotation) 
  : GameObject(_entName, _nodeName, parentNode, _physics, origin, velocity, _mass, _rest, _localInertia, rotation)
{

  if (!parentNode) {
    parentNode = mgr->getRootSceneNode();
  }

  entity = mgr->createEntity(_entName, "sphere.mesh");
  entity->setCastShadows(true);
  
  node = parentNode->createChildSceneNode(_nodeName);
  node->attachObject(entity);
  
  node->setPosition(Ogre::Vector3(origin[0], origin[1], origin[2]));
  if (rotation)
    node->setOrientation(Ogre::Quaternion((*rotation)[0], (*rotation)[1], (*rotation)[2], (*rotation)[3]));

  // Change Entity Color
  //  setColor(0,1,0,0.1,  1,1,1,0.4);

  Ogre::Vector3 s = entity->getBoundingBox().getHalfSize();
  collisionShape = new btSphereShape(s[0]);
  addToSimulator(origin, rotation);
  body->setLinearVelocity(velocity);
}
