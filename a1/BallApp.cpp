/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
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
#include <btBulletDynamicsCommon.h>
#include "BallApp.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <macUtils.h>
#   include "AppDelegate.h"
#endif

//-------------------------------------------------------------------------------------
BallApp::BallApp(void)
{
	mTimer = OGRE_NEW Ogre::Timer();
	mTimer->reset();
}
//-------------------------------------------------------------------------------------
BallApp::~BallApp(void)
{
}

void BallApp::createCamera(void) {
  BaseApplication::createCamera();
  mCamera->setPosition(0,100,900);
}

//-------------------------------------------------------------------------------------
void BallApp::createScene(void)
{
  mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.05f, 0));
  mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

  // Balls
  Ogre::Entity* eBall = mSceneMgr->createEntity("Ball", "sphere.mesh");
  eBall->setMaterialName("Ogre/Eyes");
  eBall->setCastShadows(true);
  mSceneMgr->getRootSceneNode()->createChildSceneNode("BallNode", Ogre::Vector3(0,100,0))->attachObject(eBall);

  // Environment
  
  Ogre::Plane lPlane(Ogre::Vector3::UNIT_X, 0);
  Ogre::Plane rPlane(Ogre::Vector3::NEGATIVE_UNIT_X, 0);
  Ogre::Plane bPlane(Ogre::Vector3::UNIT_Y, 0);
  Ogre::Plane tPlane(Ogre::Vector3::NEGATIVE_UNIT_Y, 0);
  Ogre::Plane farPlane(Ogre::Vector3::UNIT_Z, 0);
  Ogre::Plane nearPlane(Ogre::Vector3::NEGATIVE_UNIT_Z, 0);  

  Ogre::MeshManager::getSingleton().createPlane("ground",
                                                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                bPlane, 1500, 1500, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
  
  Ogre::Entity* eGround = mSceneMgr->createEntity("Ground", "ground");
  eGround->setMaterialName("Examples/Rockwall");
  eGround->setCastShadows(false);
  mSceneMgr->getRootSceneNode()->createChildSceneNode("GroundNode")->attachObject(eGround);
  
  // Lights
  Ogre::Light* pLight = mSceneMgr->createLight( "PointLight" );
  pLight->setType(Ogre::Light::LT_POINT);
  pLight->setPosition(-150,300,250);
  pLight->setDiffuseColour(1,0,0);
  pLight->setSpecularColour(1,0,0);

  Ogre::Light* dLight = mSceneMgr->createLight( "DirectionalLight" );
  dLight->setType(Ogre::Light::LT_DIRECTIONAL);
  dLight->setDiffuseColour(Ogre::ColourValue(.25,.25,0));
  dLight->setSpecularColour(Ogre::ColourValue(.25,.25,0));
  dLight->setDirection(Ogre::Vector3(0,-1,1));

  Ogre::Light* sLight = mSceneMgr->createLight("SpotLight");
  sLight->setType(Ogre::Light::LT_SPOTLIGHT);
  sLight->setDiffuseColour(0, 0, 1.0);
  sLight->setSpecularColour(0, 0, 1.0);
 
  sLight->setDirection(-1, -1, 0);
  sLight->setPosition(Ogre::Vector3(0, 400, 200));
 
  sLight->setSpotlightRange(Ogre::Degree(35), Ogre::Degree(50));
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
#if defined(OGRE_IS_IOS)
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        int retVal = UIApplicationMain(argc, argv, @"UIApplication", @"AppDelegate");
        [pool release];
        return retVal;
#elif (OGRE_PLATFORM == OGRE_PLATFORM_APPLE) && __LP64__
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

        mAppDelegate = [[AppDelegate alloc] init];
        [[NSApplication sharedApplication] setDelegate:mAppDelegate];
        int retVal = NSApplicationMain(argc, (const char **) argv);
        
        [pool release];
        
        return retVal;
#else
        // Create application object
        BallApp app;

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
#endif
        return 0;
    }

#ifdef __cplusplus
}
#endif
