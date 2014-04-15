#include "GameObjectDescription.h"
#include "OgreBallApplication.h"

GoalObject::GoalObject(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName,
             Ogre::SceneNode* parentNode, Physics* _physics,
             btVector3 origin, btVector3 scale, btVector3 velocity, btScalar _mass, btScalar _rest,
             btVector3 _localInertia, btQuaternion *rotation)
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, scale, velocity, _mass, _rest, _localInertia, rotation)
{

  entity = mgr->createEntity(_entName, "cube.mesh");
  entity->setCastShadows(true);

  node->attachObject(entity);
  node->scale(5, 10, 1);

  node->_update(true,true);
  node->_updateBounds();
  Ogre::Vector3 s = node->_getWorldAABB().getHalfSize();

  collisionShape = new btBoxShape(btVector3(s[0], s[1], s[2]));
  addToSimulator(Collisions::CollisionTypes::COL_GOAL,
                 Collisions::goalColliders);

  if (rotation) rotate(*rotation);
}

void GoalObject::update(float elapsedTime) {
  GameObject::update(elapsedTime);
  if(physics->checkCollisions(this)) {
    for (int i = 0; i < contexts.size(); i++) {
      if (contexts[i]->object && dynamic_cast<OgreBall*>(contexts[i]->object)) {
        OgreBallApplication::getSingleton()->activity->handleGameEnd();
      }
    }
  }
}
