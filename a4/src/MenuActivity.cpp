#include "Interpolator.h"
#include "LevelViewer.h"
#include "MenuActivity.h"
#include "SinglePlayerActivity.h"

std::vector<LevelViewer*> MenuActivity::viewerPool;

MenuActivity::MenuActivity(OgreBallApplication *app) : Activity(app) {
  selectorStart = 0;
  selectorRows = 2;
  selectorColumns = 4;
}

MenuActivity::~MenuActivity(void) {
  close();
}

void MenuActivity::close(void) {
  for (int i = 0; i < levelViewers.size(); i++)
    viewerPool.push_back(levelViewers[i]);
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
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("Menu/Background"));

  CEGUI::Window* singlePlayerButton = app->Wmgr->getWindow("Menu/SinglePlayer");
  CEGUI::Window* multiPlayerButton = app->Wmgr->getWindow("Menu/MultiPlayer");
  CEGUI::Window* quitButton = app->Wmgr->getWindow("Menu/QuitGame");

  singlePlayerButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                                     CEGUI::Event::Subscriber(&MenuActivity::SwitchToLevelSelectMenu, this));
  
  multiPlayerButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                                    CEGUI::Event::Subscriber(&MenuActivity::SwitchToMultiMenu, this));
  
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

    v->window->subscribeEvent(CEGUI::PushButton::EventMouseClick,
                              CEGUI::Event::Subscriber(&MenuActivity::StartSinglePlayer, this));

    v->setPositionPercent(0.05 + (i%selectorColumns)*0.9/selectorColumns,
                          0.2 + (i/selectorColumns)*0.6/selectorRows);
    
    levelViewers.push_back(v);
  }

  initViewers = true;
  CEGUI::System::getSingleton().setGUISheet(levelSelectorWindow);
}

bool MenuActivity::StartSinglePlayer( const CEGUI::EventArgs& e ) {
  CEGUI::MouseCursor::getSingleton().hide();

  CEGUI::String levelName = static_cast<const CEGUI::MouseEventArgs*>(&e)->window->getName();
  app->switchActivity(new SinglePlayerActivity(app, levelName.c_str()));
  return true;
}

bool MenuActivity::SwitchToMultiMenu( const CEGUI::EventArgs& e ) {
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("Menu/MultiBackground"));

  CEGUI::Window* hostButton = app->Wmgr->getWindow("Menu/Host");
  CEGUI::Window* clientButton = app->Wmgr->getWindow("Menu/Client");
  CEGUI::Window* returnButton = app->Wmgr->getWindow("Menu/Return");

  /*
    hostButton->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MenuActivity::StartHost,this));
    clientButton->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MenuActivity::StartClient,this));*/
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

