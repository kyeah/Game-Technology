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

//-------------------------------------------------------------------------------------
OgreBallApplication::OgreBallApplication(void)
{
}
//-------------------------------------------------------------------------------------
OgreBallApplication::~OgreBallApplication(void)
{
}

//-------------------------------------------------------------------------------------
void OgreBallApplication::createScene(void)
{
	mSceneMgr->setAmbientLight(Ogre::ColourValue(.5f,.5f,.5f));
	Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "sphere.mesh");
	Ogre::Entity* Ball = mSceneMgr->createEntity("Ball", "sphere.mesh");
	
   	Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("HeadNode");
	headNode->attachObject(ogreHead);
	headNode->scale(.1,.1,.1);
	
	Ogre::SceneNode* ballNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("BallNode");
	ballNode->attachObject(Ball);
	ballNode->scale(.1,.1,.1);

	Ogre::Light* light = mSceneMgr->createLight("MainLight");
	light->setPosition(20.0f, 80.0f, 50.0f);
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
