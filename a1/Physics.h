#include <btBulletDynamicsCommon.h>
#include <OgreEntity.h>

class Physics {
  btDefaultCollisionConfiguration* collisionConfiguration;
  btCollisionDispatcher* dispatcher;
  btBroadphaseInterface* overlappingPairCache;
  btSequentialImpulseConstraintSolver* solver;
  btDiscreteDynamicsWorld* dynamicsWorld;
  std::vector<btCollisionShape *> collisionShapes;
  std::map<std::string, btRigidBody *> physicsAccessors;

 public:
  Physics( btVector3 gravity = btVector3(0,-98,0) );

  btDiscreteDynamicsWorld *getDynamicsWorld() { return dynamicsWorld; }
  btRigidBody* addRigidBox(Ogre::Entity* entity, Ogre::SceneNode* node, 
                           btScalar mass=0.0f, btScalar rest=0.0f, btVector3 localInertia=btVector3(0,0,0), 
                           btVector3 origin=btVector3(0,0,0), btQuaternion *rotation=0);;
  btRigidBody* addRigidSphere(Ogre::Entity* entity, Ogre::SceneNode* node, 
                              btScalar mass=0.0f, btScalar rest=0.0f, btVector3 localInertia=btVector3(0,0,0), 
                              btVector3 origin=btVector3(0,0,0), btQuaternion *rotation=0);
  btRigidBody* addRigidBody(Ogre::Entity* entity, Ogre::SceneNode* node, btCollisionShape *rigidShape,
                            btScalar mass=0.0f, btScalar rest=0.0f, btVector3 localInertia=btVector3(0,0,0), 
                            btVector3 origin=btVector3(0,0,0), btQuaternion *rotation=0);
};