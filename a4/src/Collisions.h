#pragma once
#include <btBulletDynamicsCommon.h>
#include <vector>

class GameObject;

#define BIT(x) (1<<(x))

class Collisions {
 public:
  enum CollisionTypes {
    COL_NOTHING = 0,
    COL_LEVEL = BIT(0),
    COL_PLAYER = BIT(1),
    COL_COLLECTIBLE = BIT(2),
    COL_GOAL = BIT(3)
  };
  
  static int levelColliders, playerColliders, collectibleColliders, goalColliders;
};

class CollisionContext {
 public:
  bool hit;
  const btCollisionObject* body;
  const btCollisionObject* lastBody;
  GameObject* object;
  float distance;
  float velNorm;
  btVector3 point;
  btVector3 normal;
  btVector3 velocity;

  CollisionContext() {
    reset();
  }

  void reset() {
    hit = false;
    body = NULL;
    object = NULL;
    distance = 0.0;
    velNorm = 0.0;
    point.setZero();
    normal.setZero();
    velocity.setZero();
  }
};

class BulletContactCallback : public btCollisionWorld::ContactResultCallback {
 public:
  /* Constructor, pass whatever context you want to have available when processing contacts
   * You may also want to set m_collisionFilterGroup and m_collisionFilterMask
   * (supplied by the superclass) for needsCollision() */
 BulletContactCallback(btRigidBody& tgtBody, std::vector<CollisionContext *>& contexts)
   : btCollisionWorld::ContactResultCallback(), body(tgtBody), ctxts(contexts) { }

  btRigidBody& body;      // The body the sensor is monitoring
  std::vector<CollisionContext*>& ctxts; // External information for contact processing

  /* If you don't want to consider collisions where the bodies are joined by a constraint, override needsCollision:
   * However, if you use a btCollisionObject for #body instead of a btRigidBody,
   * then this is unnecessary; checkCollideWithOverride isn't available */
  virtual bool needsCollision(btBroadphaseProxy* proxy) const {
    // superclass will check m_collisionFilterGroup and m_collisionFilterMask
    if(!btCollisionWorld::ContactResultCallback::needsCollision(proxy))return false;
    // if passed filters, may also want to avoid contacts between constraints
    return body.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
  }

  //! Called with each contact for your own processing
  virtual btScalar addSingleResult(btManifoldPoint& cp,
                                   const btCollisionObject* colObj0, int partId0, int index0,
                                   const btCollisionObject* colObj1, int partId1, int index1) {

    CollisionContext *ctxt = new CollisionContext();

    ctxt->hit = true;
    ctxt->lastBody = ctxt->body;
    if( colObj0 == &body ) {
      ctxt->point = cp.m_localPointA;
      ctxt->body = colObj1;

    } else {
      assert(colObj1 == &body && "body does not match either collision object");
      ctxt->point = cp.m_localPointB;
      ctxt->body = colObj0;
    }

    ctxt->object = static_cast<GameObject*>(ctxt->body->getUserPointer());
    ctxt->normal = cp.m_normalWorldOnB;
    ctxt->velocity = body.getLinearVelocity();
    ctxt->velNorm = ctxt->normal.dot(ctxt->velocity);
    ctxts.push_back(ctxt);
    return 0;
  }

  // Uncomment this if using recent Bullet Physics
  virtual btScalar addSingleResult(btManifoldPoint& cp, 
                                   const btCollisionObjectWrapper* colObj0Wrap, int partId0,int index0, 
                                   const btCollisionObjectWrapper* colObj1Wrap, int partId1,int index1) {
    return addSingleResult(cp,
                           colObj0Wrap->getCollisionObject(), partId0, index0,
                           colObj1Wrap->getCollisionObject(), partId1, index1);
    
                           }

};


