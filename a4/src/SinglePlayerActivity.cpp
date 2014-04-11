#include "Interpolator.h"
#include "MenuActivity.h"
#include "SinglePlayerActivity.h"

SinglePlayerActivity::SinglePlayerActivity(OgreBallApplication *app, const char* levelName) : Activity(app) {
  MAX_TILT = .10; //Increasing this increases the maximum degree to which the level can rotate
  currTiltDelay = tiltDelay = 300;  // Increasing this increases the time it takes for the level to rotate
  lastTilt = btQuaternion(0,0,0);
  currTilt = btQuaternion(0,0,0);
  tiltDest = btQuaternion(0,0,0);
  startingLevelName = levelName;
}

SinglePlayerActivity::~SinglePlayerActivity(void) {
}

void SinglePlayerActivity::start(void) {
  CEGUI::System::getSingleton().setGUISheet(app->sheet);  // Should load our own GUI sheet for single player
  loadLevel(startingLevelName);
}

void SinglePlayerActivity::loadLevel(const char* name) {
  app->destroyAllEntitiesAndNodes();
  app->levelLoader->loadLevel(name);

  new OgreBall(app->mSceneMgr, "player1", "player1", "penguin.mesh", 0, app->mPhysics,
               app->levelLoader->playerStartPositions[0], btVector3(1,1,1), btVector3(0,0,0),
               16000.0f, 1.0f, btVector3(0,0,0), &app->levelLoader->playerStartRotations[0]);
}

bool SinglePlayerActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  return true;
}

bool SinglePlayerActivity::frameStarted( Ogre::Real elapsedTime ) {
  currTilt = Interpolator::interpQuat(currTiltDelay, elapsedTime, tiltDelay,
                                      lastTilt, tiltDest);

  app->levelLoader->levelRoot->setOrientation(Ogre::Quaternion(currTilt.w(),
                                                               currTilt.x(),
                                                               currTilt.y(),
                                                               currTilt.z()));

  return true;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::keyPressed( const OIS::KeyEvent &arg )
{
  switch(arg.key){
  case OIS::KC_D:
    tiltDest *= btQuaternion(0,0,-MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_A:
    tiltDest *= btQuaternion(0,0,MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_W:
    tiltDest *= btQuaternion(0,-MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_S:
    tiltDest *= btQuaternion(0,MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_ESCAPE:
    CEGUI::MouseCursor::getSingleton().show();
    app->switchActivity(new MenuActivity(app));
    break;
  default:
    return false;
  }

  return true;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::keyReleased( const OIS::KeyEvent &arg )
{
  switch(arg.key){
  case OIS::KC_D:
    tiltDest *= btQuaternion(0,0,MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_A:
    tiltDest *= btQuaternion(0,0,-MAX_TILT);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_W:
    tiltDest *= btQuaternion(0,MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  case OIS::KC_S:
    tiltDest *= btQuaternion(0,-MAX_TILT,0);
    lastTilt = currTilt;
    currTiltDelay = 0;
    break;
  default:
    return false;
  }

  return true;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::mouseMoved( const OIS::MouseEvent &arg )
{
  return false;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  return false;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  return false;
}
