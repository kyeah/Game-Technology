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
  entity->setMaterialName("Court/Ball");

  node->attachObject(entity);

  Ogre::Vector3 s = entity->getBoundingBox().getHalfSize();
  collisionShape = new btSphereShape(s[0]);
  addToSimulator();

  bouncedOnce = false;

  body->setCcdMotionThreshold(1);
  body->setCcdSweptSphereRadius(0.4);
}

void Ball::update(float elapsedTime) {
  static float pointsTimeDelay = 0;
  static float minZVel = 3000;

  // Prevent double taps
  pointsTimeDelay = std::max(0.0f, pointsTimeDelay - elapsedTime);

  // Cap minimum Velocity in Z direction
  btVector3 velDir = body->getLinearVelocity();
  float mag = std::max(minZVel, std::abs(velDir[2]));
  int dir = (velDir[2] > 0 ? 1 : -1);
  velDir[2] = mag*dir;
  body->setLinearVelocity(velDir);

  // Check Collisions
  if (physics->checkCollisions(this)) {
    for (int i = 0; i < contexts.size(); i++) {
      if (contexts[i]->object) {
        btVector3 point = contexts[i]->point;
        float distance = sqrt(((point.getX())*(point.getX())) + 
                              ((point.getY())*(point.getY())) + 
                              ((point.getZ() + 100) * (point.getZ() + 100)));

        distance = (distance * 128) / 215;

        Sounds::playSound(Sounds::BALL_HIT, (int)distance);

        // Check Wall hits
        Plane *p = dynamic_cast<Plane*>(contexts[i]->object);
        if (p) {
          Ogre::String name = p->getEntityName();
          int points = p->points;
          if (points > 0) {
            if (pointsTimeDelay == 0) {
              bouncedOnce = false;
              score += points;
              pointsTimeDelay = 300;
            }
          } else if (name.compare("ground") == 0) {
            if (bouncedOnce) {
              bouncedOnce = false;
              instance->restart();
            } else {
              bouncedOnce = true;
            }
          }
          break;
        }
      }
    }
  }
}
