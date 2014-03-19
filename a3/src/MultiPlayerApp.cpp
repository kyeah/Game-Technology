/*
-----------------------------------------------------------------------------
Filename:    MultiPlayerApp.cpp
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
#include "MultiPlayerApp.h"
#include "SDL_net.h"

//-------------------------------------------------------------------------------------
MultiPlayerApp::MultiPlayerApp(void)
{
}
//-------------------------------------------------------------------------------------
MultiPlayerApp::~MultiPlayerApp(void)
{
}

//-------------------------------------------------------------------------------------
void MultiPlayerApp::createScene(void)
{
	    // Set the scene's ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
 
    // Create an Entity
    Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "ogrehead.mesh");
 
    // Create a SceneNode and attach the Entity to it
    Ogre::SceneNode* headNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("HeadNode");
    headNode->attachObject(ogreHead);
 
    // Create a Light and set its position
    Ogre::Light* light = mSceneMgr->createLight("MainLight");
    light->setPosition(20.0f, 80.0f, 50.0f); 
   // create your scene here :)
}

void MultiPlayerApp::Client(void){
	char* host; //need to set
	int port; //need to set

	IPaddress ip;
	TCPsocket sd;
	int quit, len;
	char buf[512];

	SDLNet_Init();
	SDLNet_ResolveHost(&ip, host, port);
	sd = SDLNet_TCP_Open(&ip);	

	quit = 0;
 	while(!quit){
		printf("Write something\n>");
		scanf("%s", buf);
		len = strlen(buf) + 1;
		SDLNet_TCP_Send(sd, (void *)buf, len);
		if(strcmp(buf, "quit") == 0)
			quit = 1;
	}
	SDLNet_TCP_Close(sd);
	SDLNet_Quit();
}


