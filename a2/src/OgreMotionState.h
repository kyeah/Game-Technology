#ifndef __OGREMOTIONSTATE_H__
#define __OGREMOTIONSTATE_H__

#include <OgreSceneManager.h>

#include "btBulletDynamicsCommon.h"

class OgreMotionState : public btMotionState
{
 public:
  OgreMotionState ( Ogre::SceneNode * node );
  OgreMotionState ( btTransform & trans, Ogre::SceneNode * node );
  virtual ~ OgreMotionState ( void );

  void updateWorldTransform ( btTransform & pos );
  virtual void getWorldTransform ( btTransform & worldTrans ) const;
  virtual void setWorldTransform ( const btTransform & worldTrans );

 protected:
  Ogre::SceneNode * mVisibleNode;
  btTransform mWorldTransform;
};
#endif
