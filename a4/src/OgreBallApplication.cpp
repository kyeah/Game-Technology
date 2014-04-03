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

OgreBallApplication::OgreBallApplication(void)
{
  mPhysics = new Physics(btVector3(0, 0, 0));
  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
}

//-------------------------------------------------------------------------------------
OgreBallApplication::~OgreBallApplication(void)
{
}

//-------------------------------------------------------------------------------------
void OgreBallApplication::createCamera(void) {
  BaseApplication::createCamera();
  mCamera->setPosition(0,0,500);
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
  CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");                                             
  CEGUI::SchemeManager::getSingleton().create("WindowsLook.scheme");
}

//-------------------------------------------------------------------------------------
void OgreBallApplication::createScene(void)
{
	mSceneMgr->setAmbientLight(Ogre::ColourValue(.5f,.5f,.5f));
        new OgreBall(mSceneMgr, "ball", "ball", "penguin.mesh", 0, mPhysics);

	Ogre::Light* light = mSceneMgr->createLight("MainLight");
	light->setPosition(20.0f, 80.0f, 50.0f);
}

//-------------------------------------------------------------------------------------
bool OgreBallApplication::frameStarted( const Ogre::FrameEvent &evt ) {
  return BaseApplication::frameStarted(evt);
}

//-------------------------------------------------------------------------------------
bool OgreBallApplication::keyPressed( const OIS::KeyEvent &arg ) {
  return BaseApplication::keyPressed(arg);
}

bool OgreBallApplication::keyReleased( const OIS::KeyEvent &arg ) {
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
