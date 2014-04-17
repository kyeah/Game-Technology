#include "OgreBallApplication.h"
#include "GameObject.h"
#include "GameObjectDescription.h"
#include "../libs/MeshStrider.h"
#include "SinglePlayerActivity.h"

Collectible::Collectible(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, Ogre::String _nodeName, Ogre::SceneNode* parentNode,
                         Physics* _physics,
                         btVector3 origin, btVector3 scale,
                         btVector3 velocity, btScalar _mass, btScalar _rest,
                         btVector3 _localInertia, btQuaternion *rotation)
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, scale, velocity, _mass, _rest, _localInertia, rotation)
{

  entity = mgr->createEntity(_entName, _meshName);
  entity->setCastShadows(true);


  node->attachObject(entity);

  Ogre::MeshPtr meshptr = Ogre::Singleton<Ogre::MeshManager>::getSingletonPtr()->load(_meshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  MeshStrider *strider = new MeshStrider(meshptr.get());
  collisionShape = new btBvhTriangleMeshShape(strider,true,true);

  addToSimulator(Collisions::CollisionTypes::COL_COLLECTIBLE,
                 Collisions::collectibleColliders);

  setAmbient(0.5,0.0,0.0);
  setSpecular(0.1,0,0,0.4);
  if (rotation) rotate(*rotation);

  isHit = false;
}

void Collectible::update(float elapsedTime) {
  GameObject::update(elapsedTime);
  //check collisions
  if(physics->checkCollisions(this)){
    for(int i = 0; i < contexts.size(); i++){
      if(contexts[i]->object){
        OgreBall *ob = dynamic_cast<OgreBall*>(contexts[i]->object);
        if(ob && !isHit){
          isHit = true;
          std::cout << "Hit" << std::endl;
          removeFromSimulator();

          Activity *a = OgreBallApplication::getSingleton()->activity;
          if (a) a->score++;

          //TODO: Add sound
        }
      }
    }
  }
}
