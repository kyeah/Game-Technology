#include "OgreBallApplication.h"
#include "GameObject.h"
#include "GameObjectDescription.h"
#include "../libs/MeshStrider.h"
#include "SinglePlayerActivity.h"
#include "Sounds.h"
#include "HostPlayerActivity.h"
#include "Networking.h"
#include "common.h"

Collidable::Collidable(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _meshName, Ogre::String _nodeName,
                       Ogre::SceneNode* parentNode, Physics* _physics,
                       btVector3 origin, btVector3 scale, btVector3 velocity, btScalar _mass, btScalar _rest,
                       btVector3 _localInertia, btQuaternion *rotation, Ogre::String hitSound)
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, scale, velocity, _mass, _rest, _localInertia, rotation)
{

  entity = mgr->createEntity(_entName, _meshName);
  entity->setCastShadows(true);

  node->attachObject(entity);

  node->_update(true,true);
  node->_updateBounds();
  // Ogre::MeshPtr meshptr = Ogre::Singleton<Ogre::MeshManager>::getSingletonPtr()->load("cube.mesh", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  // MeshStrider *strider = new MeshStrider(meshptr.get());
  Ogre::Vector3 s = node->_getWorldAABB().getHalfSize();

  collisionShape = new btBoxShape(btVector3(s[0], s[1], s[2]));

  addToSimulator(Collisions::CollisionTypes::COL_COLLIDABLE,
                 Collisions::collidableColliders);

  if (rotation) rotate(*rotation);

  mHitSound = hitSound;

}

void Collidable::update(float elapsedTime) {
  GameObject::update(elapsedTime);
  //check collisions
  if(physics->checkCollisions(this)){
    for(int i = 0; i < contexts.size(); i++){
      if(contexts[i]->object){
        OgreBall *ob = dynamic_cast<OgreBall*>(contexts[i]->object);
        if(ob){
          Sounds::playSoundEffect(mHitSound.c_str(), (Sounds::MAX_VOLUME));
        }
      }
    }
  }
}
