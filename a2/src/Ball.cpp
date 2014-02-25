#include "RacquetApp.h"
#include "RacquetObject.h"
#include "Sounds.h"

Ball::Ball(Ogre::SceneManager *mgr, Ogre::String _entName, Ogre::String _nodeName, Ogre::SceneNode* parentNode, 
           Physics* _physics,
           btVector3 origin, btVector3 velocity, btScalar _mass, btScalar _rest, 
           btVector3 _localInertia, btQuaternion *rotation) 
  : GameObject(mgr, _entName, _nodeName, parentNode, _physics, origin, velocity, _mass, _rest, _localInertia, rotation)
{
  
  entity = mgr->createEntity(_entName, "sphere.mesh");
  entity->setCastShadows(true);
  
  node->attachObject(entity);
  
  // Change Entity Color
  //  setColor(0,1,0,0.1,  1,1,1,0.4);
  
  Ogre::Vector3 s = entity->getBoundingBox().getHalfSize();
  collisionShape = new btSphereShape(s[0]);
  addToSimulator();
  
  bouncedOnce = false;
  
  body->setCcdMotionThreshold(1);
  body->setCcdSweptSphereRadius(0.4);
}

void Ball::update(float elapsedTime) {
  if (physics->checkCollisions(this)) {
    Sounds::playSound(Sounds::BALL_HIT); 
    
    for (int i = 0; i < contexts.size(); i++) {
      if (contexts[i]->object) {
        Ogre::String name = contexts[i]->object->getEntityName();
        if (name.compare("farWall") == 0) {
          score++;
          break;
        } else if (name.compare("nearWall") == 0) {
          bouncedOnce = false;
          score = 0;
          break;
        } else if (name.compare("ground") == 0) {
          if (bouncedOnce) {
            // setColor(1,1,1,0,  1,1,1,0);
            bouncedOnce = false;
            score = 0;
            break;
          } else {
            // setColor(1,0,0,1,  1,0,0,1);
            bouncedOnce = true;
            break;
          }
        }
      }
    }
  }
}
