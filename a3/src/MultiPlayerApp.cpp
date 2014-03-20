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

btVector3 racquetInitialPosition(0,700.0f,0);
btVector3 playerInitialPosition(100, -1200, -2245);
static bool pongMode = false;
Ogre::Light* scene_lights[6];

//-------------------------------------------------------------------------------------
MultiPlayerApp::MultiPlayerApp(void)
{
	connected = false;
	mPhysics = new Physics(btVector3(0, -7000, 0));
        MultiPlayerApp::Connect();
}
//-------------------------------------------------------------------------------------
MultiPlayerApp::~MultiPlayerApp(void)
{
         MultiPlayerApp::Close();
}

//-------------------------------------------------------------------------------------
void MultiPlayerApp::createCamera(void){
	BaseApplication::createCamera();
	mCamera->setPosition(0,0,7000);
	mCamera->lookAt(0,0,500);
}

void MultiPlayerApp::createScene(void)
{
   printf("in create scene\n");
	    // Set the scene's ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
    mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
 
	  // Boxed Environment
  Ogre::Plane planes[] = {
    Ogre::Plane(Ogre::Vector3::UNIT_X, 0),
    Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_X, 0),
    Ogre::Plane(Ogre::Vector3::UNIT_Y, 0),
    Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Y, 0),
    Ogre::Plane(Ogre::Vector3::UNIT_Z, 0),
    Ogre::Plane(Ogre::Vector3::NEGATIVE_UNIT_Z, 0)
  };

  std::string pNames[] = {
    "leftWall", "rightWall", "ground", "ceiling", "nearWall", "farWall"
  };

  Ogre::Vector3 up[] = {
    Ogre::Vector3::UNIT_Y, Ogre::Vector3::UNIT_Y,
    Ogre::Vector3::UNIT_Z, Ogre::Vector3::UNIT_Z,
    Ogre::Vector3::UNIT_X, Ogre::Vector3::UNIT_X
  };

  //w applies to leftWall/rightWall
  //h applies to ground/ceiling
  //l applies to farWall/nearWall
  int w, l, h;
  int worldLength, worldWidth, worldHeight;
  worldLength = l = 5000;
  worldWidth = worldHeight = w = h = 4500;

  btVector3 pos[] = {
    btVector3(-w/2,0,0),
    btVector3(w/2,0,0),
    btVector3(0,-h/2,0),
    btVector3(0,h/2,0),
    btVector3(0,0,-l/2),
    btVector3(0,0,l/2)
  };

  int width, height;
  width = w;
  height = h;
  for (int i = 0; i < 6; i++) {
    if(pNames[i] == "leftWall" || pNames[i] == "rightWall") { width = l; height = w; }
    if(pNames[i] == "ground" || pNames[i] == "ceiling") { width = w; height = l;}
    if(pNames[i] == "farWall" || pNames[i] == "nearWall") { width = w; height = w;}
    Ogre::MeshManager::getSingleton().createPlane(pNames[i],
                                                  Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  planes[i], width, height, 20, 20, true, 1, 5, 5, up[i]);

    Plane *p = new Plane(mSceneMgr, pNames[i], pNames[i], pNames[i], 0, mPhysics, pos[i]);

    if (pNames[i] == "ground") {
      p->getEntity()->setMaterialName("Court/Floor");
    } else {
      p->getEntity()->setMaterialName("Court/Wall");
    }

    if (pNames[i] == "farWall") p->points = 1;
  }

  Ogre::MeshManager::getSingleton().createPlane("2wall",
                                                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                planes[5], w/4, w/4, 20, 20, true, 1, 5, 5, up[5]);

  Ogre::MeshManager::getSingleton().createPlane("4wall",
                                                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                planes[5], w/6, w/6, 20, 20, true, 1, 5, 5, up[5]);

  // Lights
  Ogre::Light* lights[9];
  int z;
  for(z = 0; z < 9; z++) {
    std::stringstream ss;
    ss << "point light" << z;
    lights[z] = mSceneMgr->createLight(ss.str());
    lights[z]->setType(Ogre::Light::LT_POINT);
    lights[z]->setDiffuseColour(.1,.1,.1);
    lights[z]->setSpecularColour(.1,.1,.1);
    
    ss << z;
    if (z < 6) {
      scene_lights[z] = mSceneMgr->createLight(ss.str());
      scene_lights[z]->setType(Ogre::Light::LT_SPOTLIGHT);
      scene_lights[z]->setDiffuseColour(rand(),rand(),rand());
      scene_lights[z]->setSpecularColour(rand(),rand(),rand());
      scene_lights[z]->setDirection(rand(), rand(), rand());
      scene_lights[z]->setSpotlightOuterAngle(Ogre::Radian(0.1));
    }
  }
  
  lights[0]->setPosition(-1499,1499,0);
  lights[1]->setPosition(-1499,1499,1000);
  lights[2]->setPosition(-1499,1499,2000);
  lights[3]->setPosition(-1000,1499,2499);
  lights[4]->setPosition(0,1499,2499);
  lights[5]->setPosition(1000,1499,2499);
  lights[6]->setPosition(1499,1499,2000);
  lights[7]->setPosition(1499,1499,1000);
  lights[8]->setPosition(1499,1499,0);

  // Front Wall
  scene_lights[0]->setDirection(-0.5,0,1);
  scene_lights[1]->setDirection(0.5,0,1);
  scene_lights[0]->setPosition(0,1800,0);
  scene_lights[1]->setPosition(0,1800,0);

  // Right Wall
  scene_lights[2]->setDirection(-1,0,0);
  scene_lights[3]->setDirection(-1,0,-1);
  scene_lights[2]->setPosition(0,-1800,0);
  scene_lights[3]->setPosition(0,-1800,0);
  
  // Left Wall
  scene_lights[4]->setDirection(1,0,0);
  scene_lights[5]->setDirection(1,0,-1);
  scene_lights[4]->setPosition(0,-1800,0);
  scene_lights[5]->setPosition(0,-1800,0);


  mPlayer = new Dude(mSceneMgr, "Player", "PlayerNode", 0, mPhysics,
                     playerInitialPosition, btVector3(0,0,0), 0);
  mRacquet = new Racquet(mSceneMgr, "Racquet", "Racquetnode", mPlayer->getNode(), mPhysics,
                         racquetInitialPosition);
  mBall = new Ball(mSceneMgr, "Ball", "BallNode", 0, mPhysics,
                   btVector3(100,100,150),
                   btVector3( rand() % 120 - 60, rand() % 80 - 40, 6000),
                   1000);

  printf("setting ball position initially\n");
  mBall->setPosition(btVector3(0,0,0));

  mBall->getNode()->attachObject(mSceneMgr->createParticleSystem("fountain1", "Examples/PurpleFountain"));
  mBall->getNode()->attachObject(mSceneMgr->createParticleSystem("fountain2", "Examples/PurpleFountain"));
  //if (pongMode) mPlayer->getEntity()->setVisible(false);

}

void MultiPlayerApp::Connect(){
	 SDLNet_Init();
        if(SDLNet_ResolveHost(&ip, NULL, 65501) == -1) {
                printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
                exit(0);
        }
        sd = SDLNet_TCP_Open(&ip);
        if(!sd){
                printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
                exit(0);
        }

        csd = SDLNet_TCP_Accept(sd);
        while(!csd){
                csd = SDLNet_TCP_Accept(sd);
                printf("trying to accept...\n");
                if(csd){
                        remoteIP = SDLNet_TCP_GetPeerAddress(csd);
                        if(remoteIP){
                                printf("Successfully connected to %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
                                connected = true;
                        }
                }
        }

}

char* MultiPlayerApp::Receive(){
	for(int i = 0; i < 512; i++){
		buf[i] = ' ';	
	}	
	if(SDLNet_TCP_Recv(csd, buf, 512) > 0){
		return (char*)buf;
	}
	return "error";
}

void MultiPlayerApp::Close(){
	SDLNet_TCP_Close(csd);
	SDLNet_TCP_Close(sd);
	SDLNet_Quit();
	connected = false;
}

void MultiPlayerApp::createFrameListener(void){
	BaseApplication::createFrameListener();
}

bool MultiPlayerApp::keyReleased(const OIS::KeyEvent &arg){
  static bool vert = false;

  switch(arg.key){
  case OIS::KC_R:
//    restart();
    return true;
  case OIS::KC_P:
/*    swing = unswing = 0;
    pongMode = !pongMode;
    mPlayer->setOrientation(btQuaternion(0,0,0,1));
    mPlayer->setPosition(playerInitPos);
    mPlayer->getEntity()->setVisible(!mPlayer->getEntity()->isVisible());
*/    return true;
  case OIS::KC_J:
  case OIS::KC_D:
//    mDirection -= btVector3(-40, 0, 0);
//    oDirection.x -= -40;
    return true;
  case OIS::KC_S:
    if (vert) {
//      mDirection -= btVector3(0, -40, 0);
//      oDirection.y -= -40;
    } else {
//      mDirection -= btVector3(0, 0, -40);
//      oDirection.z -= -40;
    }
    return true;
  case OIS::KC_A:
//    mDirection -= btVector3(40, 0, 0);
//    oDirection.x -= 40;
    return true;
  case OIS::KC_W:
    if (vert) {
//      mDirection -= btVector3(0, 40, 0);
//      oDirection.y -= 40;
    } else {
//        mDirection -= btVector3(0, 0, 40);
//        oDirection.z -= 40;
    }
    return true;
  case OIS::KC_LSHIFT:
//    movementSpeed = 1;
    return true;
  case OIS::KC_G:
/*    static int gravity = 0;
    btDiscreteDynamicsWorld *world = mPhysics->getDynamicsWorld();
    if (gravity == 0) {
      world->setGravity(btVector3(0,0,0));
      mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Off");
    } else if (gravity == 1) {
      world->setGravity(btVector3(0,gravMag,0));
      mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Upward");
    } else if (gravity == 2) {
      world->setGravity(btVector3(0,-gravMag,0));
      mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Downward");                                                                                                     
    }

    gravity = (gravity+1)%3;
*/    return true;
  }

  return BaseApplication::keyPressed(arg);
}

bool MultiPlayerApp::mouseMoved( const OIS::MouseEvent& arg ) {
/*  if (swing == 0 && unswing == 0) {
    int x = arg.state.X.rel;
    int y = arg.state.Y.rel;

    static float rotfactor = 6.28 / 1800;

    if (pongMode) {
      //Boundaries
      if(x < 0 && mRacquet->getPosition().getX() >= 2000){
        x = 0;
      }
      if(x > 0 && mRacquet->getPosition().getX() <= -2000){
        x = 0;
      }
      if(y < 0 && mRacquet->getPosition().getY() >= 2000){
        y = 0;
      }
      if(y > 0 && mRacquet->getPosition().getY() <= -2000){
        y = 0;
      }



      mPlayer->translate(btVector3(-x,-y,0));
    } else {
      mPlayer->rotate(btQuaternion(btVector3(0,0,1), btScalar(x*rotfactor)));
    }
  }
*/
}

bool MultiPlayerApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id ) {
/*  if(swing == 0 && unswing == 0) {

    if(id == OIS::MB_Left || id == OIS::MB_Right) {
      swing = SWING_DELAY;
      Sounds::playSound(Sounds::RACQUET_SWOOSH, 100);

      Ogre::Vector3 p = mRacquet->getNode()->getPosition();
      axis = new btVector3(p[1], -p[0], 0);

      right_mouse_button = (id == OIS::MB_Right);
    }
  }
*/
  return BaseApplication::mouseReleased(arg, id);
}



bool MultiPlayerApp::keyPressed( const OIS::KeyEvent &arg ) {
  static bool vert = false;

  switch(arg.key){
  case OIS::KC_D:
//    mDirection += btVector3(-40, 0, 0);
//    oDirection.x += -40;
    return true;
  case OIS::KC_S:
    if (vert) {
//      mDirection += btVector3(0, -40, 0);
//      oDirection.y += -40;
    } else {
//      mDirection += btVector3(0, 0, -40);
//      oDirection.z += -40;
    }
    return true;
  case OIS::KC_A:
//    mDirection += btVector3(40, 0, 0);
//    oDirection.x += 40;
    return true;
  case OIS::KC_W:
    if (vert) {
//      mDirection += btVector3(0, 40, 0);
//      oDirection.y += 40;
    } else {
//        mDirection += btVector3(0, 0, 40);
//        oDirection.z += 40;
    }

    return true;
  case OIS::KC_LSHIFT:
//    movementSpeed = 2;
    return true;
  case OIS::KC_SPACE:
//    if(swing == 0 && unswing == 0) {
//      swing = SWING_DELAY;
//      Sounds::playSound(Sounds::RACQUET_SWOOSH, 100);
//    }
    return true;
  }

  return BaseApplication::keyPressed(arg);
}



bool MultiPlayerApp::frameStarted(const Ogre::FrameEvent &evt) {
	char* msg = MultiPlayerApp::Receive();
	int x,y,z; 
	sscanf(msg, "Ball %d %d %d", &x, &y, &z);
	mBall->setPosition(btVector3(x,y,z));
        sscanf(msg, "Player %d %d %d", &x,&y,&z);	
	mPlayer->setPosition(btVector3(x,y,z));

	return true;
}
