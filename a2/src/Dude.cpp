#include "RacquetApp.h"
#include "RacquetObject.h"
#include "Sounds.h"


Dude::Dude(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName, Ogre::SceneNode* parentNode,
           Physics* _physics,
           btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest,
           btVector3 _localInertia, btQuaternion *rotation)
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, velocity, _mass, _rest, _localInertia, rotation)
{

  entity = mgr->createEntity(_entName, "sphere.mesh");
  entity->setCastShadows(true);

  node->attachObject(entity);

  Ogre::Vector3 s = entity->getBoundingBox().getHalfSize();
  collisionShape = new btSphereShape(s[0]);
  addToSimulator();

  body->setCcdMotionThreshold(1);
  body->setCcdSweptSphereRadius(0.4);
}

void Dude::update(float elapsedTime) {
  if (physics->checkCollisions(this)) {
    for (int i = 0; i < contexts.size(); i++) {
      if (contexts[i]->object) {
        Ogre::String name = contexts[i]->object->getEntityName();
      }
    }
  }
}    

