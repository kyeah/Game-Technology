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
#include "MenuActivity.h"
#include "SinglePlayerActivity.h"

using namespace Ogre;
using namespace std;
using namespace sh;



OgreBallApplication::OgreBallApplication(void)
{
  mPhysics = new Physics(btVector3(0, -1960, 0));
  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
}

//-------------------------------------------------------------------------------------
OgreBallApplication::~OgreBallApplication(void)
{
}

//-------------------------------------------------------------------------------------
void OgreBallApplication::destroyAllEntitiesAndNodes(void) {
  mPhysics->removeAllObjects();
  mSceneMgr->destroyAllEntities();
  mSceneMgr->getRootSceneNode()->removeAndDestroyAllChildren();
  mSceneMgr->destroyAllLights();
  // mSceneMgr->destroyAllParticleSystems();
  // mSceneMgr->destroyAllRibbonTrails();

  levelRoot = mSceneMgr->getRootSceneNode()->createChildSceneNode("root");
  levelLoader->levelRoot = levelRoot;
  levelLoader->clearKnobs();
}

//-------------------------------------------------------------------------------------
void OgreBallApplication::switchActivity(Activity *activity) {
  this->activity = activity;
  destroyAllEntitiesAndNodes();
  activity->start();
}

//-------------------------------------------------------------------------------------
void OgreBallApplication::createScene(void)
{
  levelLoader = new LevelLoader(mSceneMgr, mCamera, mPhysics, levelRoot);
  levelLoader->loadResources("media/OgreBall/scripts");
  switchActivity(new MenuActivity(this));
}

void OgreBallApplication::createCamera(void) {
  BaseApplication::createCamera();
  mCamera->lookAt(0,0,0);
}

void OgreBallApplication::loadResources(void) {
  BaseApplication::loadResources();

  // Initialize CEGUI
  CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
  CEGUI::Font::setDefaultResourceGroup("Fonts");
  CEGUI::Scheme::setDefaultResourceGroup("Schemes");
  CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
  CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

  mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
  CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
  CEGUI::SchemeManager::getSingleton().create("WindowsLook.scheme");
  CEGUI::SchemeManager::getSingleton().create("VanillaSkin.scheme");
  // CEGUI::SchemeManager::getSingleton().create("GameGUI.scheme");
  CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook", "MouseArrow"); 
 
  Wmgr = &CEGUI::WindowManager::getSingleton();
  Wmgr->loadWindowLayout("Menu.layout");
  Wmgr->loadWindowLayout("MultiSubMenu.layout");
  Wmgr->loadWindowLayout("Chatbox.layout");
  sheet = Wmgr->createWindow("DefaultWindow", "CEGUIDemo/Sheet");
  // CEGUI::System::getSingleton().setGUISheet(sheet);
}

//-------------------------------------------------------------------------------------
bool OgreBallApplication::frameStarted( const Ogre::FrameEvent &evt ) {
  bool result =  BaseApplication::frameStarted(evt);
  static Ogre::Real time = mTimer->getMilliseconds();

  Ogre::Real elapsedTime = mTimer->getMilliseconds() - time;
  time = mTimer->getMilliseconds();

  if (mPhysics) mPhysics->stepSimulation(elapsedTime);
  activity->frameStarted(elapsedTime);

  return result;
}

bool OgreBallApplication::frameRenderingQueued( const Ogre::FrameEvent &evt ) {
  if(mWindow->isClosed())
    return false;

  if(mShutDown)
    return false;

  //Need to capture/update each device
  mKeyboard->capture();
  mMouse->capture();

  mTrayMgr->frameRenderingQueued(evt);
  if (!mTrayMgr->isDialogVisible()) {
    mCameraMan->frameRenderingQueued(evt);
  }

  activity->frameRenderingQueued(evt);
  
  return true;
}

//-------------------------------------------------------------------------------------
bool OgreBallApplication::keyPressed( const OIS::KeyEvent &arg ) {
  if (!activity->keyPressed(arg)) {
    return BaseApplication::keyPressed(arg);
  }
  return true;
}

bool OgreBallApplication::keyReleased( const OIS::KeyEvent &arg ) {
  if (!activity->keyReleased(arg)) {
    return BaseApplication::keyReleased(arg);
  }
  return true;
}

bool OgreBallApplication::mouseMoved( const OIS::MouseEvent &arg ) {
  if (!activity->mouseMoved(arg)) {
    return BaseApplication::mouseMoved(arg);
  }
  return true;
}

bool OgreBallApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  return activity->mousePressed(arg, id);
}

bool OgreBallApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
  if (!activity->mouseReleased(arg, id)) {
    return BaseApplication::mouseReleased(arg, id);
  }
  return true;
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
