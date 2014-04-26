#include "Interpolator.h"
#include "LevelViewer.h"
#include "MenuActivity.h"
#include "SinglePlayerActivity.h"
#include "ClientPlayerActivity.h"
#include "HostPlayerActivity.h"

#define NOT_SELECTED -1
#define SINGLE_PLAYER 0
#define MULTI_HOST 1
#define MULTI_CLIENT 2

std::vector<LevelViewer*> MenuActivity::viewerPool;

int type_flag = NOT_SELECTED;

MenuActivity::MenuActivity(OgreBallApplication *app) : Activity(app) {
  selectorStart = 0;
  selectorRows = 2;
  selectorColumns = 4;
}

MenuActivity::~MenuActivity(void) {
  close();
}

void MenuActivity::close(void) {
  for (int i = 0; i < levelViewers.size(); i++) {
    viewerPool.push_back(levelViewers[i]);
  }
  levelViewers.clear();
}

void MenuActivity::start(void) {
  app->levelLoader->loadLevel("menuBG");

  new OgreBall(app->mSceneMgr, "free", "penguin", "penguin.mesh", 0, app->mPhysics,
               app->levelLoader->playerStartPositions[0]);

  const CEGUI::EventArgs* args;
  SwitchToMainMenu(*args);
}

void MenuActivity::handleGameEnd() {

}

bool MenuActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);
  return true;
}

bool MenuActivity::frameStarted( Ogre::Real elapsedTime ) {
  for (int i = 0; i < levelViewers.size(); i++) {
    levelViewers[i]->frameStarted(elapsedTime);
  }

  return true;
}

bool MenuActivity::SwitchToMainMenu( const CEGUI::EventArgs& e ) {
  type_flag = NOT_SELECTED;
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("Menu/Background"));

  CEGUI::Window* singlePlayerButton = app->Wmgr->getWindow("Menu/SinglePlayer");
  CEGUI::Window* multiPlayerButton = app->Wmgr->getWindow("Menu/MultiPlayer");
  CEGUI::Window* quitButton = app->Wmgr->getWindow("Menu/QuitGame");

  singlePlayerButton->removeEvent(CEGUI::PushButton::EventClicked);
  singlePlayerButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                                     CEGUI::Event::Subscriber(&MenuActivity::SinglePlayerLevelSelectWrapper, this));

  multiPlayerButton->removeEvent(CEGUI::PushButton::EventClicked);
  multiPlayerButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                                    CEGUI::Event::Subscriber(&MenuActivity::SwitchToMultiMenu, this));

  quitButton->removeEvent(CEGUI::PushButton::EventClicked);
  quitButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&MenuActivity::quit,this));
}

bool MenuActivity::SwitchToHostSelectMenu( const CEGUI::EventArgs& e){
	CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("Menu/Hosts"));	

	CEGUI::ScrollablePane* panel = static_cast<CEGUI::ScrollablePane*>(app->Wmgr->getWindow("Menu/ScrollablePane"));
	//SCROLLBAR DOESN'T SHOW?	
	panel->setShowVertScrollbar(true);
	CEGUI::Scrollbar* scrollbarVertical = static_cast<CEGUI::Scrollbar*>(app->Wmgr->getWindow("Menu/VerticalScrollbar"));
	panel->addChildWindow(scrollbarVertical);
	CEGUI::Window* quitButton = app->Wmgr->getWindow("Menu/ExitGame");


	//string x = temp->getText();
	CEGUI::Window* temp;
	for(int i = 0; i < 184; i++){
		temp = app->Wmgr->getWindow("hostbutton" + std::to_string(i));
		panel->addChildWindow(temp);
		temp->subscribeEvent(CEGUI::PushButton::EventMouseClick, 
				CEGUI::Event::Subscriber(&MenuActivity::StartMultiPlayerClient, this));
	}

    	quitButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&MenuActivity::quit,this));
	
	

}

bool MenuActivity::SwitchToLevelSelectMenu( const CEGUI::EventArgs& e ) {
  CEGUI::WindowManager *wmgr = CEGUI::WindowManager::getSingletonPtr();

  // Find level selector sheet
  if (wmgr->isWindowPresent("levelSelector")) {
    levelSelectorWindow = wmgr->getWindow("levelSelector");
  } else {
    levelSelectorWindow = wmgr->createWindow("DefaultWindow", "levelSelector");
  }

  LevelLoader *loader = LevelLoader::getSingleton();

  // This variable ensures that viewers are only initiated once, then reused.
  static bool initViewers = false;

  // Load each level in level viewer
  int selectorEnd = selectorStart + (selectorRows*selectorColumns);
  for (int i = selectorStart; i < selectorEnd && i < loader->levelNames.size(); i++) {
    LevelViewer *v;

    if (initViewers) {
      // Recycle your Level Viewers!
      if (viewerPool.empty()) break;
      v = viewerPool.back();
      viewerPool.pop_back();
      v->loadLevel(loader->levelNames[i].c_str());
    } else {
      v = new LevelViewer(app->mRenderer, loader->levelNames[i].c_str());
    }

    levelSelectorWindow->addChildWindow(v->window);

    if(type_flag == SINGLE_PLAYER){
	v->window->removeEvent(CEGUI::PushButton::EventMouseClick);
    	v->window->subscribeEvent(CEGUI::PushButton::EventMouseClick,
                              CEGUI::Event::Subscriber(&MenuActivity::StartSinglePlayer, this));
    }
    else if (type_flag == MULTI_HOST){
	v->window->removeEvent(CEGUI::PushButton::EventMouseClick);
	v->window->subscribeEvent(CEGUI::PushButton::EventMouseClick, 
			      CEGUI::Event::Subscriber(&MenuActivity::StartMultiPlayerHost, this));
    }
    v->setPositionPercent(0.05 + (i%selectorColumns)*0.9/selectorColumns,
                          0.2 + (i/selectorColumns)*0.6/selectorRows);

    levelViewers.push_back(v);
  }

  initViewers = true;
  CEGUI::System::getSingleton().setGUISheet(levelSelectorWindow);
}

bool MenuActivity::SinglePlayerLevelSelectWrapper( const CEGUI::EventArgs& e ){
	type_flag = SINGLE_PLAYER;
	MenuActivity::SwitchToLevelSelectMenu(e);
}

bool MenuActivity::MultiPlayerLevelSelectWrapper( const CEGUI::EventArgs& e ){
	type_flag = MULTI_HOST;
	MenuActivity::SwitchToLevelSelectMenu(e);
}

bool MenuActivity::StartSinglePlayer( const CEGUI::EventArgs& e ) {
  CEGUI::MouseCursor::getSingleton().hide();

  CEGUI::String levelName = static_cast<const CEGUI::MouseEventArgs*>(&e)->window->getName();
  LevelLoader *loader = LevelLoader::getSingleton();
  for(int i = 0; i < loader->levelNames.size(); i++)
  {
    
  }
  app->switchActivity(new SinglePlayerActivity(app, levelName.c_str()));
  return true;
}

bool MenuActivity::StartMultiPlayerHost( const CEGUI::EventArgs& e ){
	CEGUI::MouseCursor::getSingleton().hide();

	CEGUI::String levelName = static_cast<const CEGUI::MouseEventArgs*>(&e)->window->getName();
	app->switchActivity(new HostPlayerActivity(app, levelName.c_str()));
	return true;
	
}

bool MenuActivity::StartMultiPlayerClient( const CEGUI::EventArgs& e) {
	CEGUI::MouseCursor::getSingleton().hide();
	CEGUI::String hostName = static_cast<const CEGUI::MouseEventArgs*>(&e)->window->getText();
	printf("host name is %s\n", hostName.c_str());	
	app->switchActivity(new ClientPlayerActivity(app, hostName.c_str()));
	return true;
}

bool MenuActivity::SwitchToMultiMenu( const CEGUI::EventArgs& e ) {
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("Menu/MultiBackground"));

  CEGUI::Window* hostButton = app->Wmgr->getWindow("Menu/Host");
  CEGUI::Window* clientButton = app->Wmgr->getWindow("Menu/Client");
  CEGUI::Window* returnButton = app->Wmgr->getWindow("Menu/Return");

  hostButton->subscribeEvent(CEGUI::PushButton::EventClicked,
    	CEGUI::Event::Subscriber(&MenuActivity::MultiPlayerLevelSelectWrapper,this));
  clientButton->subscribeEvent(CEGUI::PushButton::EventClicked,
    	CEGUI::Event::Subscriber(&MenuActivity::SwitchToHostSelectMenu,this));
  /*
    hostButton->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MenuActivity::StartHost,this));
    clientButton->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MenuActivity::StartClient,this));*/

  returnButton->removeEvent(CEGUI::PushButton::EventClicked);
  returnButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                               CEGUI::Event::Subscriber(&MenuActivity::SwitchToMainMenu, this));
}

bool MenuActivity::quit( const CEGUI::EventArgs& e ) {
  app->mShutDown = true;
}

//-------------------------------------------------------------------------------------

bool MenuActivity::keyPressed( const OIS::KeyEvent &arg )
{
  CEGUI::System &sys = CEGUI::System::getSingleton();
  sys.injectKeyDown(arg.key);
  sys.injectChar(arg.text);
  return true;
}

//-------------------------------------------------------------------------------------

bool MenuActivity::keyReleased( const OIS::KeyEvent &arg )
{
  CEGUI::System::getSingleton().injectKeyUp(arg.key);
  return true;
}

//-------------------------------------------------------------------------------------

bool MenuActivity::mouseMoved( const OIS::MouseEvent &arg )
{
  CEGUI::System &sys = CEGUI::System::getSingleton();
  sys.injectMouseMove(arg.state.X.rel, arg.state.Y.rel);
  // Scroll wheel.
  if (arg.state.Z.rel)
    sys.injectMouseWheelChange(arg.state.Z.rel / 120.0f);
  return true;
}

//-------------------------------------------------------------------------------------

bool MenuActivity::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  CEGUI::System::getSingleton().injectMouseButtonDown(OgreBallApplication::convertButton(id));
  return true;
}

//-------------------------------------------------------------------------------------

bool MenuActivity::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  CEGUI::System::getSingleton().injectMouseButtonUp(OgreBallApplication::convertButton(id));
  return true;
}
