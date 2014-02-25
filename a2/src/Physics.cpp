#include "Physics.h"
#include "GameObject.h"
#include "RacquetObject.h"
#include "OgreMotionState.h"
#include "RacquetApp.h"
#include "SDL.h"
#include "SDL_mixer.h"

int Physics::simID = 0;

Physics::Physics( btVector3 gravity ) {
  collisionConfiguration = new btDefaultCollisionConfiguration();
  dispatcher = new btCollisionDispatcher(collisionConfiguration);
  overlappingPairCache = new btDbvtBroadphase();
  solver = new btSequentialImpulseConstraintSolver();
  dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
  dynamicsWorld->setGravity(gravity);
}

void Physics::stepSimulation(const Ogre::Real elapsedTime, int maxSubSteps, const Ogre::Real fixedTimeStep) {
  dynamicsWorld->stepSimulation(elapsedTime, maxSubSteps, fixedTimeStep);

  // Update Game state
  for (int i = 0; i < objList.size(); i++) {
    objList[i]->updateTransform();
    objList[i]->update(elapsedTime);
  }
}

bool Physics::checkCollisions(GameObject *obj) {
  if (obj->getContactCallback()) {
    std::vector<CollisionContext *> *contexts = obj->getCollisionContexts();
    BulletContactCallback callback = *(obj->getContactCallback());
    
    contexts->clear();
    dynamicsWorld->contactTest(obj->getBody(), callback);
	
    //play sound if something is hit
/*    if(!contexts->empty()){
	RacquetApp::PlayHitSound();
    }*/ 
    return !(contexts->empty());
  }
  return false;
}

bool Physics::checkCollisionPair(GameObject *obj1, GameObject *obj2) {
  if (obj1->getContactCallback()) {
    BulletContactCallback callback = *(obj1->getContactCallback());
    obj1->getCollisionContexts()->clear();
    dynamicsWorld->contactPairTest(obj1->getBody(), obj2->getBody(), callback);
    return !(obj1->getCollisionContexts()->empty());
  }
  return false;
}

/******************************/

btRigidBody* Physics::addRigidBox(Ogre::Entity* entity, Ogre::SceneNode* node,
                                  btScalar mass, btScalar rest, btVector3 localInertia, btVector3 origin, btQuaternion *rotation) {
  Ogre::Vector3 s = entity->getBoundingBox().getHalfSize();
  btCollisionShape *boxShape = new btBoxShape( btVector3(s[0],s[1],s[2]) );
  addRigidBody(entity, node, boxShape, mass, rest, localInertia, origin, rotation);

};
btRigidBody* Physics::addRigidSphere(Ogre::Entity* entity, Ogre::SceneNode* node,
                                     btScalar mass, btScalar rest, btVector3 localInertia, btVector3 origin, btQuaternion *rotation) {
  Ogre::Vector3 s = entity->getBoundingBox().getHalfSize();
  btCollisionShape *sphereShape = new btSphereShape( btScalar(s[0]) );
  addRigidBody(entity, node, sphereShape, mass, rest, localInertia, origin, rotation);
};

btRigidBody* Physics::addRigidBody(Ogre::Entity* entity, Ogre::SceneNode* node, btCollisionShape *rigidShape,
                                   btScalar mass, btScalar rest, btVector3 localInertia, btVector3 origin, btQuaternion *rotation) {
  btTransform startTransform;
  startTransform.setIdentity();
  startTransform.setOrigin(origin);
  if (rotation) {
    startTransform.setRotation(*rotation);
  }

  rigidShape->calculateLocalInertia(mass, localInertia);

  // Instantiate the body and add it to the dynamics world
  btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);

  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, rigidShape, localInertia);
  btRigidBody *body = new btRigidBody(rbInfo);
  body->setRestitution(rest);
  body->setUserPointer(node);

  getDynamicsWorld()->addRigidBody(body);
  return body;
}

int Physics::addObject(GameObject *obj) {
  objList.push_back(obj);
  obj->setSimID(simID);
  getDynamicsWorld()->addRigidBody(obj->getBody());

  return simID++;
}
