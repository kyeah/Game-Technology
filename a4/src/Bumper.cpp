#include "OgreBallApplication.h"
#include "GameObject.h"
#include "GameObjectDescription.h"
#include "../libs/MeshStrider.h"
#include "SinglePlayerActivity.h"
#include "Sounds.h"

Bumper::Bumper(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName,
             Ogre::SceneNode* parentNode, Physics* _physics,
             btVector3 origin, btVector3 scale, btVector3 velocity, btScalar _mass, btScalar _rest,
             btVector3 _localInertia, btQuaternion *rotation, Ogre::String hitSound)
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, scale, velocity, _mass, _rest, _localInertia, rotation)
{

	entity = mgr->createEntity(_entName, "cube.mesh");
  	entity->setCastShadows(true);

  	node->attachObject(entity);

  	Ogre::MeshPtr meshptr = Ogre::Singleton<Ogre::MeshManager>::getSingletonPtr()->load("cube.mesh", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    MeshStrider *strider = new MeshStrider(meshptr.get());
    collisionShape = new btBvhTriangleMeshShape(strider,true,true);

    addToSimulator(Collisions::CollisionTypes::COL_BUMPER,
                 Collisions::collectibleColliders);

    if (rotation) rotate(*rotation);

  	mHitSound = hitSound;

}

void Bumper::update(float elapsedTime) {
  GameObject::update(elapsedTime);
  //check collisions
  if(physics->checkCollisions(this)){
    for(int i = 0; i < contexts.size(); i++){
      if(contexts[i]->object){
        OgreBall *ob = dynamic_cast<OgreBall*>(contexts[i]->object);
        if(ob){
          std::cout << "Hit" << std::endl;
        }
      }
    }
  }
}