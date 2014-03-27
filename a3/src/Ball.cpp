#include <pthread.h>

#include "RacquetApp.h"
#include "RacquetObject.h"
#include "Sounds.h"
#include "common.h"

struct args{
	Ogre::Entity *entity;
	Plane *p;
};

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

  isMultiplayer = false;
  bouncedOnce = false;
  lastHit = 1;

  body->setCcdMotionThreshold(1);
  body->setCcdSweptSphereRadius(0.4);
}

void *Ball::changeWall(void *params){

	struct args *arg = (struct args*)params;
	Ogre::Entity *ent = (Ogre::Entity*)arg->entity;
	Plane* plane = (Plane*)arg->p;
  	Ogre::String name = plane->getEntityName();
	
	if(name.compare("ground") != 0){	
		ent->setMaterialName("Court/HitWall");
		usleep(1000);
		ent->setMaterialName("Court/Wall");	
	}
}
void Ball::setMultiplayer(bool yesOrNo){
  isMultiplayer = yesOrNo;
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
    if(isMultiplayer){
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
            btVector3 pos = this->getPosition();
            int whichTeam = 2;
            if (pos.getZ() >= 600)
              whichTeam = 1;
            ScoringPlane *sp = dynamic_cast<ScoringPlane*>(p);

            if (!sp) {
              struct args *arg = (struct args*)malloc(sizeof(struct args*));
              arg->entity = p->getEntity();
              arg->p = p;
              pthread_t thread;
              pthread_create(&thread, 0, changeWall, (void*)(arg)); 
            }

            Ogre::String name = p->getEntityName();
            int points = p->points;
            if (points > 0) {
              if (pointsTimeDelay == 0) {
                if(whichTeam == 1)
                  team1Score += points;
                else
                  team2Score += points;
                if (points > 1) {
                  sp->cycleColor();
                  Sounds::playSound(Sounds::SCORE_POINT, (int)distance);
                }
                pointsTimeDelay = 300;
              }
            } 
            break;
          }
        }
      }
    }
    else {
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
            ScoringPlane *sp = dynamic_cast<ScoringPlane*>(p);

            if (!sp) {
              struct args *arg = (struct args*)malloc(sizeof(struct args*));
              arg->entity = p->getEntity();
              arg->p = p;
              pthread_t thread;
              pthread_create(&thread, 0, changeWall, (void*)(arg)); 
            }

  	  Ogre::String name = p->getEntityName();
            int points = p->points;
            if (points > 0) {
              if (pointsTimeDelay == 0) {
                bouncedOnce = false;
                score += points;
                if (points > 1) {
                  sp->cycleColor();
                  Sounds::playSound(Sounds::SCORE_POINT, (int)distance);
                }
                pointsTimeDelay = 300;
              }
            } else if (name.compare("ground") == 0) {
              if (bouncedOnce) {
                bouncedOnce = false;
                lastscore = score;
                score = 0;
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
}
