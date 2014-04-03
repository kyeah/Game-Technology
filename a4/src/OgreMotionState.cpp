#include "OgreMotionState.h"

OgreMotionState::OgreMotionState ( Ogre::SceneNode * node )
{
  mVisibleNode = node;
  Ogre::Vector3 position = mVisibleNode -> _getDerivedPosition();
  Ogre::Quaternion quaternion = mVisibleNode -> _getDerivedOrientation();
  mWorldTransform.setOrigin ( btVector3 ( position.x, position.y, position.z ) );
  mWorldTransform.setRotation ( btQuaternion ( quaternion.x, quaternion.y, quaternion.z, quaternion.w ) );
}

OgreMotionState::OgreMotionState ( btTransform & trans, Ogre::SceneNode * node )
{
  mVisibleNode = node;
  mWorldTransform = trans;
}

OgreMotionState::~OgreMotionState(void)
{
}

void OgreMotionState::updateWorldTransform( btTransform & pos ) {
  mWorldTransform = pos;
}

void OgreMotionState::getWorldTransform ( btTransform & worldTrans ) const
{
  worldTrans = mWorldTransform;
}

void OgreMotionState::setWorldTransform ( const btTransform & worldTrans )
{
  if ( mVisibleNode )
    {
      btQuaternion quaternion = worldTrans.getRotation ( );
      btVector3 position = worldTrans.getOrigin ( );
      mVisibleNode -> _setDerivedOrientation ( Ogre::Quaternion ( quaternion.getW ( ), quaternion.getX ( ),
                                                                  quaternion.getY ( ), quaternion.getZ ( ) ) );
      mVisibleNode -> _setDerivedPosition ( Ogre::Vector3 ( position.x ( ), position.y ( ), position.z ( ) ) );
      mWorldTransform = worldTrans;
    }
}
