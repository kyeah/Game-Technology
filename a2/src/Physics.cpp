#include "Physics.h"
#include "GameObject.h"
#include "RacquetObject.h"

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
    std::cout << "start update" << std::endl;
    Racquet *r = static_cast<Racquet *>(objList[i]);
    std::cout << "test update" << std::endl;
    r->update(elapsedTime);
    //    objList[i]->update(elapsedTime);
    std::cout << "end update" << std::endl;
  }

  /*
  // Update OGRE node positions
  btAlignedObjectArray<btCollisionObject*> objs = dynamicsWorld->getCollisionObjectArray();
  for (int i = 0; i < objs.size(); i++) {
    btCollisionObject *obj = objs[i];
    btRigidBody *body = btRigidBody::upcast(obj);
    
    if (body && body->getMotionState()) {
      btTransform trans;
      body->getMotionState()->getWorldTransform(trans);
      
      void *userPointer = body->getUserPointer();
      if (userPointer) {
        btQuaternion orientation = trans.getRotation();
        GameObject *gameObject = static_cast<GameObject *>(userPointer);
        Ogre::SceneNode *sceneNode = gameObject->getNode();
        sceneNode->setPosition(Ogre::Vector3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()));
        sceneNode->setOrientation(Ogre::Quaternion(orientation.getW(), orientation.getX(), orientation.getY(), orientation.getZ()));
      }
    }
  }
  */
}

bool Physics::checkCollisions(GameObject *obj) {
  std::cout << "physics::checkcollisions" << std::endl;
  if (obj->getContactCallback()) {
    std::cout << "physics::checkcollisions okay" << std::endl;
    std::vector<CollisionContext *> *contexts = obj->getCollisionContexts();
    contexts->clear();
    std::cout << "physics::checkcollisions okay 2" << std::endl;
    dynamicsWorld->contactTest(obj->getBody(), *(obj->getContactCallback()));
    return !(contexts->empty());
  }
  return false;
}

bool Physics::checkCollisionPair(GameObject *obj1, GameObject *obj2) {
  if (obj1->getContactCallback()) {
    obj1->getCollisionContexts()->clear();
    dynamicsWorld->contactPairTest(obj1->getBody(), obj2->getBody(), *(obj1->getContactCallback()));
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
  static int simID;
  objList.push_back(obj);
  obj->setSimID(simID);
  getDynamicsWorld()->addRigidBody(obj->getBody());
  
  return ++simID;
}
