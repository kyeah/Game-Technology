#include <OgreMeshManager.h>
#include "GameObjectDescription.h"
#include "../libs/MeshStrider.h"

MeshObject::MeshObject(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, Ogre::String _nodeName,
             Ogre::SceneNode* parentNode, Physics* _physics,
             btVector3 origin, btVector3 scale, btVector3 velocity, btScalar _mass, btScalar _rest,
             btVector3 _localInertia, btQuaternion *rotation)
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, scale, velocity, _mass, _rest, _localInertia, rotation)
{

  entity = mgr->createEntity(_entName, _meshName);
  // entity->setCastShadows(true);

  node->attachObject(entity);
  
  Ogre::MeshPtr meshptr = Ogre::Singleton<Ogre::MeshManager>::getSingletonPtr()->load(_meshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  MeshStrider *strider = new MeshStrider(meshptr.get());
  collisionShape = new btBvhTriangleMeshShape(strider,true,true);
  addToSimulator(Collisions::CollisionTypes::COL_LEVEL,
                 Collisions::levelColliders);
  if (rotation) rotate(*rotation);
}

void MeshObject::update(float elapsedTime) { 
  GameObject::update(elapsedTime);
}
