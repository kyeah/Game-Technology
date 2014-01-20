#include "Physics.h"

Physics::Physics( btVector3 gravity ) {
  collisionConfiguration = new btDefaultCollisionConfiguration();
  dispatcher = new btCollisionDispatcher(collisionConfiguration);
  overlappingPairCache = new btDbvtBroadphase();
  solver = new btSequentialImpulseConstraintSolver();
  dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
  dynamicsWorld->setGravity(gravity);
}

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
