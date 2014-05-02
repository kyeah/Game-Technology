#pragma once
#include <btBulletDynamicsCommon.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>

class GameObject;

class Physics {
 public:
  btDefaultCollisionConfiguration* collisionConfiguration;
  btCollisionDispatcher* dispatcher;
  btBroadphaseInterface* overlappingPairCache;
  btSequentialImpulseConstraintSolver* solver;
  btDiscreteDynamicsWorld* dynamicsWorld;
  std::deque<GameObject*> objList;
  std::vector<btCollisionShape *> collisionShapes;
  std::map<std::string, btRigidBody *> physicsAccessors;

  static int simID;

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

  int addObject(GameObject *obj);
  int addObject(GameObject *obj, short group, short mask);

  void removeObject(GameObject *obj);
  bool checkCollisions(GameObject *obj);
  bool checkCollisionPair(GameObject *obj1, GameObject *obj2);
  void stepSimulation(const Ogre::Real elapsedTime, int maxSubsteps = 1,
                      const Ogre::Real fixedTimestep = 1.0f/60.0f);

  std::deque<GameObject*>& getObjects();
  void removeAllObjects();
  int indexOfObject(GameObject *obj);
};
