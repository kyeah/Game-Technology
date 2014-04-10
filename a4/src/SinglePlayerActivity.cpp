#include "MenuActivity.h"
#include "SinglePlayerActivity.h"

SinglePlayerActivity::SinglePlayerActivity(OgreBallApplication *app) : Activity(app) {
  xTilt = 0;
  zTilt = 0;
  totalXTilt = 0;
  totalZTilt = 0;
  ROTATION_FACTOR = 0.005; //Increasing this increases the speed at which the level rotates on key press
  MAX_TILT = .15; //Increasing this increases the maximum degree to which the level can rotate
}

SinglePlayerActivity::~SinglePlayerActivity(void) {
}

void SinglePlayerActivity::start(void) {
  CEGUI::System::getSingleton().setGUISheet(app->sheet);
  //  loadLevel("baseLevel");
  loadLevel("clearPath");
}

void SinglePlayerActivity::loadLevel(char* name) {
  app->destroyAllEntitiesAndNodes();
  app->levelLoader->loadLevel(name);

  new OgreBall(app->mSceneMgr, "player1", "player1", "penguin.mesh", 0, app->mPhysics,
               app->levelLoader->playerStartPositions[0], btVector3(1,1,1), btVector3(0,0,0),
               160.0f, 1.0f, btVector3(0,0,0), &app->levelLoader->playerStartRotations[0]);
}

bool SinglePlayerActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  return true;
}

bool SinglePlayerActivity::frameStarted( Ogre::Real elapsedTime ) {

  if(zTilt != 0){
    btVector3 axis = btVector3(0, 0, 1);
    if (zTilt > 0 && totalZTilt >= MAX_TILT){}
    else if(zTilt < 0 && totalZTilt <= -1 * MAX_TILT){}
    else {
      app->levelLoader->rotateLevel(&axis, zTilt);
      totalZTilt += zTilt;
    }
  } 
  if(xTilt != 0){
    btVector3 axis = btVector3(1, 0, 0);
    if (xTilt > 0 && totalXTilt >= MAX_TILT){}
    else if(xTilt < 0 && totalXTilt <= -1 * MAX_TILT){}
    else{
      app->levelLoader->rotateLevel(&axis, xTilt);
      totalXTilt += xTilt;
    }
  }

  return true;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::keyPressed( const OIS::KeyEvent &arg )
{
  switch(arg.key){
  case OIS::KC_D:
    zTilt += ROTATION_FACTOR;
    break;
  case OIS::KC_A:
    zTilt += -1.0 * ROTATION_FACTOR;
    break;
  case OIS::KC_W:
    xTilt += -1.0 * ROTATION_FACTOR;
    break;
  case OIS::KC_S:
    xTilt += ROTATION_FACTOR;
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
    zTilt -= ROTATION_FACTOR;
    break;
  case OIS::KC_A:
    zTilt -= -1.0 * ROTATION_FACTOR;
    break;
  case OIS::KC_W:
    xTilt -= -1.0 * ROTATION_FACTOR;
    break;
  case OIS::KC_S:
    xTilt -= ROTATION_FACTOR;
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
