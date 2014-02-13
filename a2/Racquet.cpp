#include "RacquetObject.h"

Racquet::Racquet(Ogre::SceneManager *mgr, Ogre::SceneNode* parentNode, Physics* _physics,
           btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest, 
           btVector3 _localInertia, btQuaternion *rotation) {
  
  static int ballID;
  
  std::stringstream ss;
  ss << "myRacquet" << ballID;
  std::string ent = ss.str();
  ss << "node";
  ballID++;
  
  Racquet(mgr, ent, ss.str(), parentNode, _physics, origin, velocity, _mass, _rest, _localInertia, rotation);

}

Racquet::Racquet(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName, Ogre::SceneNode* parentNode, 
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
  
  node->scale(.5,3,3);

  // Change Entity Color
  // setColor(0,1,0,0.1,  0.5,1,1,0.4);

  Ogre::Vector3 s = entity->getBoundingBox().getHalfSize();
  collisionShape = new btBoxShape( btVector3(s[0],s[1],s[2]) );
  body = addToSimulator(origin, rotation);
  body->setLinearVelocity(velocity);
}
