#include <btBulletDynamicsCommon.h>

class Physics {
  btDefaultCollisionConfiguration* collisionConfiguration;
  btCollisionDispatcher* dispatcher;
  btBroadphaseInterface* overlappingPairCache;
  btSequentialImpulseConstraintSolver* solver;
  btDiscreteDynamicsWorld* dynamicsWorld;
  std::vector<btCollisionShape *> collisionShapes;
  std::map<std::string, btRigidBody *> physicsAccessors;

 public:
  Physics(void) {
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    overlappingPairCache = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0,-9.8,0));
  }
  
  btDiscreteDynamicsWorld *getDynamicsWorld() { return dynamicsWorld; }
};
