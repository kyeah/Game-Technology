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
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>

#include "BaseMultiplayerApp.h"
#include "RacquetObject.h"
#include "Sounds.h"
#include "SDL_net.h"
#include "Networking.h"
#include "common.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <macUtils.h>
#   include "AppDelegate.h"
#endif

#include <iostream>
#include <string>

//-------------------------------------------------------------------------------------
BaseMultiplayerApp::BaseMultiplayerApp(void) {
  mPhysics = new Physics(btVector3(0,-gravMag,0));
  mTimer = OGRE_NEW Ogre::Timer();
  mTimer->reset();
  MAX_SPEED = btScalar(8000);
  Sounds::init();
  connected = false;
  chatFocus = false;
  allowKeyRelease = true;
}

//-------------------------------------------------------------------------------------
BaseMultiplayerApp::~BaseMultiplayerApp(void)
{
  Networking::Close();
}

void BaseMultiplayerApp::createCamera(void) {
  BaseApplication::createCamera();
  mCamera->setPosition(0,0,-7000);
  mCamera->lookAt(0,0,500);
}

void BaseMultiplayerApp::createFrameListener(void) {
  BaseApplication::createFrameListener();

  Ogre::StringVector items;
  items.push_back("Highscore");
  items.push_back("");
  items.push_back("Last Score");
  items.push_back("Current Score");
  items.push_back("Gravity");

  mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
  mDetailsPanel->setParamValue(DETAILS_HIGHSCORE, "0");
  mDetailsPanel->setParamValue(DETAILS_LASTSCORE, "0");
  mDetailsPanel->setParamValue(DETAILS_SCORE, "0");
  mDetailsPanel->setParamValue(DETAILS_GRAVITY, "Downwards");
}

void BaseMultiplayerApp::restart() {
  /*  static int gamenum = 0;
      mPhysics->removeObject(mBall);
      mBall->setPosition(btVector3(0,0,0));
      mBall->setVelocity(btVector3(0,0,0));
      mBall->updateTransform();
      mBall->addToSimulator();
      std::cout << "Game " << gamenum++ << ": " << score << std::endl;
      lastscore = score;
      score = 0;*/
}

Player* BaseMultiplayerApp::findPlayer(int userID) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (players[i] && players[i]->getId() == userID) {
      return players[i];
    }
  }
  return NULL;
}

Player* BaseMultiplayerApp::addPlayer(int userID) {
  Player* mPlayer = new Player(userID);

  std::stringstream ss;
  ss << "Player" << userID;
  std::string playerEnt = ss.str();
  ss << "node";

  std::stringstream ssr;
  ssr << "Racquet" << userID;
  std::string racquetEnt = ssr.str();
  ssr << "node";

  mPlayer->setNode(new Dude(mSceneMgr, playerEnt, ss.str(), 0, mPhysics,
                            playerInitialPositions[userID], btVector3(0,0,0), 0));

  mPlayer->setRacquet(new Racquet(mSceneMgr, racquetEnt, ssr.str(), mPlayer->getNode()->getNode(), mPhysics,
                                  racquetInitPos));

  players[userID] = mPlayer;
  return mPlayer;
}

void BaseMultiplayerApp::toggleChat() {
 chatFocus = !chatFocus;
  chatEditBox->setVisible(!chatEditBox->isVisible());
  if (chatFocus)
    chatEditBox->activate();
  else
    chatEditBox->deactivate();
}

void BaseMultiplayerApp::addChatMessage(const char* msg) {
  CEGUI::ListboxTextItem* chatItem;
  if(chatBox->getItemCount() == 7)
    {
      chatItem = static_cast<CEGUI::ListboxTextItem*>(chatBox->getListboxItemFromIndex(0));
      chatItem->setAutoDeleted(false);
      chatBox->removeItem(chatItem);
      chatItem->setAutoDeleted(true);
      chatItem->setText(msg);
    }
  else
    {
      // Create a new listbox item
      chatItem = new CEGUI::ListboxTextItem(msg);
    }
  chatBox->addItem(chatItem);
  chatBox->ensureItemIsVisible(chatBox->getItemCount());
}

void BaseMultiplayerApp::createNewScoringPlane(int points, btVector3 pos, btVector3 speed, btVector3 linearFactor, btVector3 angularFactor) {
  static int wallID;
  std::stringstream ss;
  ss << points << "wall";
  std::string mesh = ss.str();
  ss << wallID;
  std::string ent = ss.str();
  ss << "node";
  std::string node = ss.str();
  wallID++;

  ScoringPlane *extra = new ScoringPlane(worldWidth, worldLength, worldHeight,
                                         mSceneMgr, ent, mesh, node, 0, mPhysics,
                                         pos, speed, 0.0, 1.0);

  extra->points = points;

  extra->cycleColor();
  extra->getBody()->setLinearFactor(linearFactor);
  extra->getBody()->setAngularFactor(angularFactor);
}

//-------------------------------------------------------------------------------------
void BaseMultiplayerApp::createScene(void)
{
  mSceneMgr->setAmbientLight(Ogre::ColourValue(.5f, .5f, .5f));
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

//  Ogre::MeshManager::getSingleton().createPlane("2wall-p2", 
//						Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
//						planes[4], w/4, w/4, 20, 20, true, 1, 5, 5, up[4]);

  //Ogre::MeshManager::getSingleton().createPlane("4wall-p2", 
//						Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
//						planes[4], w/6, w/6, 20, 20, true, 1, 5, 5, up[4]);
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
      discolights[z] = mSceneMgr->createLight(ss.str());
      discolights[z]->setType(Ogre::Light::LT_SPOTLIGHT);
      discolights[z]->setDiffuseColour(rand(),rand(),rand());
      discolights[z]->setSpecularColour(rand(),rand(),rand());
      discolights[z]->setDirection(rand(), rand(), rand());
      discolights[z]->setSpotlightOuterAngle(Ogre::Radian(0.1));
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
  discolights[0]->setDirection(-0.5,0,1);
  discolights[1]->setDirection(0.5,0,1);
  discolights[0]->setPosition(0,1800,0);
  discolights[1]->setPosition(0,1800,0);

  // Right Wall
  discolights[2]->setDirection(-1,0,0);
  discolights[3]->setDirection(-1,0,-1);
  discolights[2]->setPosition(0,-1800,0);
  discolights[3]->setPosition(0,-1800,0);

  // Left Wall
  discolights[4]->setDirection(1,0,0);
  discolights[5]->setDirection(1,0,-1);
  discolights[4]->setPosition(0,-1800,0);
  discolights[5]->setPosition(0,-1800,0);

  Player *mPlayer = addPlayer(myId);

  mBall = new Ball(mSceneMgr, "Ball", "BallNode", 0, mPhysics,
                   btVector3(100,100,150),
                   btVector3( rand() % 120 - 60, rand() % 80 - 40, 6000),
                   1000);

  mBall->getNode()->attachObject(mSceneMgr->createParticleSystem("fountain1", "Examples/PurpleFountain"));
  mBall->getNode()->attachObject(mSceneMgr->createParticleSystem("fountain2", "Examples/PurpleFountain"));

  if (mPlayer->pongMode) mPlayer->getNode()->getEntity()->setVisible(false);

  createNewScoringPlane(2, btVector3( 0, rand() % 3500 - 2000, 5000/2 - 5));
  createNewScoringPlane(4, btVector3( 0, rand() % 3500 - 2000, 5000/2 - 5), btVector3(30,0,0));

  // Initialize CEGUI
  mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
  CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
  CEGUI::SchemeManager::getSingleton().create("WindowsLook.scheme");
  CEGUI::SchemeManager::getSingleton().create("VanillaSkin.scheme");
  CEGUI::SchemeManager::getSingleton().create("GameGUI.scheme");

  CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window *sheet = wmgr.createWindow("DefaultWindow", "CEGUIDemo/Sheet");
  chat = wmgr.loadWindowLayout("Chatbox.layout");
  chatBox = (CEGUI::Listbox*)wmgr.getWindow("ConsoleRoot/ChatBox");
  chatEditBox = (CEGUI::Editbox*)wmgr.getWindow("ConsoleRoot/EditBox");

  chat->setPosition(CEGUI::UVector2(CEGUI::UDim(0.0f, 0),
                                    CEGUI::UDim(0.65f, 0)));
  sheet->addChildWindow(chat);
  CEGUI::System::getSingleton().setGUISheet(sheet);
  chatEditBox->setVisible(false);
}

bool BaseMultiplayerApp::frameStarted(const Ogre::FrameEvent &evt) {
  for (int i = 0; i < 2; i++) {
    Ogre::Vector3 v = discolights[i]->getDirection();
    double x = v[0] + 0.02;
    if (x > 1.0) x = x - 2.0;
    discolights[i]->setDirection(x, v[1], v[2]);
  }
  for (int i = 2; i < 6; i++) {
    Ogre::Vector3 v = discolights[i]->getDirection();
    double z = v[2] + 0.02;
    if (z > 1.0) z = z - 2.0;
    discolights[i]->setDirection(v[0], v[1], z);
  }
}
