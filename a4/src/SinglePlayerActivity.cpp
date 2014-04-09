#include "SinglePlayerActivity.h"

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
               8.0f, 1.0f, btVector3(0,0,0), &app->levelLoader->playerStartRotations[0]);
}

// The world is your oyster...
bool SinglePlayerActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  return true;
}

bool SinglePlayerActivity::frameStarted( Ogre::Real elapsedTime ) {
  return true;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::keyPressed( const OIS::KeyEvent &arg )
{
  return false;
}

//-------------------------------------------------------------------------------------

bool SinglePlayerActivity::keyReleased( const OIS::KeyEvent &arg )
{
  return false;
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
