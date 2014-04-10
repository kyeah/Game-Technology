/*
  -----------------------------------------------------------------------------
  Filename:    OgreBallApplication.cpp
  -----------------------------------------------------------------------------

  This source file is part of the
  ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
  //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
  / \_// (_| | | |  __/  \  /\  /| |   <| |
  \___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
  |___/
  Tutorial Framework
  http://www.ogre3d.org/tikiwiki/
  -----------------------------------------------------------------------------
*/
#include "OgreBallApplication.h"

using namespace Ogre;
using namespace std;
using namespace sh;



OgreBallApplication::OgreBallApplication(void)
{
  mPhysics = new Physics(btVector3(0, -490, 0));
  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
  xTilt = 0;
  zTilt = 0;
  totalXTilt = 0;
  totalZTilt = 0;
  ROTATION_FACTOR = 0.0001; //Increasing this increases the speed at which the level rotates on key press
  MAX_TILT = .1; //Increasing this increases the maximum degree to which the level can rotate
  MAX_SPEED = btScalar(100); //Increasing this increases the max speed at which the player ball can move
}

//-------------------------------------------------------------------------------------
OgreBallApplication::~OgreBallApplication(void)
{
}

//-------------------------------------------------------------------------------------
void OgreBallApplication::createScene(void)
{
  levelLoader = new LevelLoader(mSceneMgr, mCamera, mPhysics);
  levelLoader->loadResources("media/OgreBall/scripts");
  levelLoader->loadLevel("baseLevel");

  mPlayer = new OgreBall(mSceneMgr, "player1", "player1", "penguin.mesh", 0, mPhysics, 
               levelLoader->playerStartPositions[0]);
}

void OgreBallApplication::createCamera(void) {
  BaseApplication::createCamera();
  mCamera->lookAt(0,0,0);
}

void OgreBallApplication::createFrameListener(void) {
  BaseApplication::createFrameListener();

  // Initialize CEGUI
  CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
  CEGUI::Font::setDefaultResourceGroup("Fonts");
  CEGUI::Scheme::setDefaultResourceGroup("Schemes");
  CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
  CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

  mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
  //  CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
  //  CEGUI::SchemeManager::getSingleton().create("WindowsLook.scheme");
}

//-------------------------------------------------------------------------------------
bool OgreBallApplication::frameStarted( const Ogre::FrameEvent &evt ) {
  bool result =  BaseApplication::frameStarted(evt);
  static Ogre::Real time = mTimer->getMilliseconds();

  Ogre::Real elapsedTime = mTimer->getMilliseconds() - time;
  time = mTimer->getMilliseconds();

  if (mPhysics) mPhysics->stepSimulation(elapsedTime);
  //limit ball velocity
  playerVelocity = mPlayer->getBody()->getLinearVelocity();
  btScalar speed = playerVelocity.length();
  if(speed > MAX_SPEED){
    playerVelocity *= MAX_SPEED/speed;
    mPlayer->getBody()->setLinearVelocity(playerVelocity);
  }

  //Tilt the level 
  if(zTilt != 0){
    btVector3 axis = btVector3(0, 0, 1);
    if (zTilt > 0 && totalZTilt >= MAX_TILT){}
    else if(zTilt < 0 && totalZTilt <= -1 * MAX_TILT){}
    else{
      levelLoader->rotateLevel(&axis, zTilt);
      totalZTilt += zTilt;
    }
  }
  if(xTilt != 0){
    btVector3 axis = btVector3(1, 0, 0);
    if (xTilt > 0 && totalXTilt >= MAX_TILT){}
    else if(xTilt < 0 && totalXTilt <= -1 * MAX_TILT){}
    else{
      levelLoader->rotateLevel(&axis, xTilt);
      totalXTilt += xTilt;
    }
  }
  return result;
}

//-------------------------------------------------------------------------------------
bool OgreBallApplication::keyPressed( const OIS::KeyEvent &arg ) {

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
  }
  return BaseApplication::keyPressed(arg);
}

bool OgreBallApplication::keyReleased( const OIS::KeyEvent &arg ) {
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
  }
  return BaseApplication::keyReleased(arg);
}

bool OgreBallApplication::mouseMoved( const OIS::MouseEvent &arg ) {
  return BaseApplication::mouseMoved(arg);
}
bool OgreBallApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  return BaseApplication::mouseReleased(arg, id);
}


#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
  {
    // Create application object
    OgreBallApplication app;

    try {
      app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
      MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
      std::cerr << "An exception has occured: " <<
        e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
  }

#ifdef __cplusplus
}
#endif
