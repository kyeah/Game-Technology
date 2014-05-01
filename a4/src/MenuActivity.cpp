#include "Interpolator.h"
#include "LevelViewer.h"
#include "MenuActivity.h"
#include "SinglePlayerActivity.h"
#include "ClientPlayerActivity.h"
#include "HostPlayerActivity.h"
#include "Leaderboard.h"
#include "OBAnimationManager.h"
#include "Sounds.h"
#include "Networking.h"

#define NOT_SELECTED -1
#define SINGLE_PLAYER 0
#define MULTI_HOST 1
#define MULTI_CLIENT 2

std::vector<LevelViewer*> MenuActivity::viewerPool;

int type_flag = NOT_SELECTED;
int player_flag = 0;

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

bool MenuActivity::quit( const CEGUI::EventArgs& e ) {
  app->mShutDown = true;
}

void MenuActivity::start(void) {
  Sounds::playBackground("media/OgreBall/sounds/Menu.mp3", 64);

  // Load a background
  app->levelLoader->loadLevelRand();

  new OgreBall(app->mSceneMgr, "free", "penguin", "penguin.mesh", 0, app->mPhysics,
               app->levelLoader->playerStartPositions[0]);

  // Pre-load windows
  CEGUI::WindowManager *wmgr = CEGUI::WindowManager::getSingletonPtr();

  // Level Selector
  levelSelectorWindow = wmgr->getWindow("Menu/LevelSelector");
  lsBack = wmgr->getWindow("LevelSelector/Back");
  lsPrev = wmgr->getWindow("LevelSelector/Prev");
  lsNext = wmgr->getWindow("LevelSelector/Next");

  lsBack->subscribeEvent(CEGUI::PushButton::EventClicked,
                         CEGUI::Event::Subscriber(&MenuActivity::SwitchToMainMenu, this));

  lsPrev->subscribeEvent(CEGUI::PushButton::EventClicked,
                         CEGUI::Event::Subscriber(&MenuActivity::handleLSPrev, this));

  lsNext->subscribeEvent(CEGUI::PushButton::EventClicked,
                         CEGUI::Event::Subscriber(&MenuActivity::handleLSNext, this));

  // Server List
  serverListWindow = wmgr->getWindow("Menu/ServerList");
  serverListbox = (CEGUI::Listbox*)wmgr->getWindow("ServerList/List");
  serverListBack = wmgr->getWindow("ServerList/Back");

  serverListbox->setMultiselectEnabled(false);
  serverListbox->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
                                CEGUI::Event::Subscriber(&MenuActivity::PromptForJoinServer, this));

  serverListBack->subscribeEvent(CEGUI::PushButton::EventClicked,
                                 CEGUI::Event::Subscriber(&MenuActivity::SwitchToMultiMenu, this));

  // Prompt
  promptWindow = wmgr->getWindow("Prompt");
  promptHeader = wmgr->getWindow("Prompt/Header");
  promptInputbox = (CEGUI::Editbox*)wmgr->getWindow("Prompt/Input");
  promptSubmit = wmgr->getWindow("Prompt/Submit");
  promptCancel = wmgr->getWindow("Prompt/Cancel");

  promptCancel->subscribeEvent(CEGUI::PushButton::EventClicked,
                               CEGUI::Event::Subscriber(&MenuActivity::CancelPrompt, this));

  for (int i = 0; i < 8; i++) {
    std::stringstream ss;
    ss << "LevelSelector/" << i+1;
    lsButtons[i] = wmgr->getWindow(ss.str());
  }

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

/*
  ==========================================================
  = Main Menu
  ==========================================================
*/

bool MenuActivity::SwitchToMainMenu( const CEGUI::EventArgs& e ) {
  selectorStart = 0;
  close();

  type_flag = NOT_SELECTED;
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("Menu/Background"));

  CEGUI::Window* singlePlayerButton = app->Wmgr->getWindow("Menu/SinglePlayer");
  CEGUI::Window* multiPlayerButton = app->Wmgr->getWindow("Menu/MultiPlayer");
  CEGUI::Window* quitButton = app->Wmgr->getWindow("Menu/QuitGame");

  singlePlayerButton->removeEvent(CEGUI::PushButton::EventClicked);
  singlePlayerButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                                     CEGUI::Event::Subscriber(&MenuActivity::SwitchToPlayerSelectMenu, this));

  multiPlayerButton->removeEvent(CEGUI::PushButton::EventClicked);
  multiPlayerButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                                    CEGUI::Event::Subscriber(&MenuActivity::SwitchToMultiMenu, this));

  quitButton->removeEvent(CEGUI::PushButton::EventClicked);
  quitButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&MenuActivity::quit,this));
}

/*
  ==========================================================
  = Host Selection Menu
  ==========================================================
*/

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

/*
  ==========================================================
  = Multiplayer Menu
  ==========================================================
*/

bool MenuActivity::SwitchToMultiMenu( const CEGUI::EventArgs& e ) {
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("Menu/MultiBackground"));

  CEGUI::Window* hostButton = app->Wmgr->getWindow("Menu/Host");
  CEGUI::Window* clientButton = app->Wmgr->getWindow("Menu/Client");
  CEGUI::Window* returnButton = app->Wmgr->getWindow("Menu/Return");

  hostButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&MenuActivity::MultiPlayerLevelSelectWrapper,this));
  /*  clientButton->subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&MenuActivity::SwitchToHostSelectMenu,this));
  */
  clientButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                               CEGUI::Event::Subscriber(&MenuActivity::SwitchToServerListMenu,this));

  /*
    hostButton->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MenuActivity::StartHost,this));
    clientButton->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MenuActivity::StartClient,this));*/

  returnButton->removeEvent(CEGUI::PushButton::EventClicked);
  returnButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                               CEGUI::Event::Subscriber(&MenuActivity::SwitchToMainMenu, this));
}

bool MenuActivity::SwitchToServerListMenu( const CEGUI::EventArgs& e ) {
  CEGUI::System::getSingleton().setGUISheet(serverListWindow);

  // Ping all known hosts from file
  vector<PingResponseMessage*> responses = Networking::hostCheck( "data/hosts/hosts.txt" );

  // Display list of active hosts waiting to start a game
  serverListbox->resetList();

  for (int i = 0; i < responses.size(); i++) {
    CEGUI::ListboxTextItem* chatItem = new CEGUI::ListboxTextItem(responses[i]->lobbyName);
    chatItem->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
    chatItem->setUserData(responses[i]);
    serverListbox->addItem(chatItem);
  };

  serverListbox->ensureItemIsVisible((size_t)0);
}

bool MenuActivity::JoinServer( const CEGUI::EventArgs& e ) {
  CEGUI::String name = promptInputbox->getText();
  if (!name.length()) return true;

  CEGUI::System::getSingleton().getGUISheet()->removeChildWindow(promptWindow);

  CEGUI::ListboxItem * selectedItem = serverListbox->getFirstSelectedItem();

  int id;
  char levelname[128];

  PingResponseMessage* serverData = static_cast<PingResponseMessage*>(selectedItem->getUserData());
  std::cout << "connecting to " << serverData->hostName << std::endl;

  ConnectAck *ack = new ConnectAck();
  if (Networking::clientConnect(ack, name.c_str(), serverData->hostName)) {
    std::cout << "Client connected" << std::endl;
    app->switchActivity(new ClientPlayerActivity(app, ack));
  }
}

bool MenuActivity::PromptForJoinServer( const CEGUI::EventArgs& e ) {
  CEGUI::System::getSingleton().getGUISheet()->addChildWindow(promptWindow);
  promptHeader->setText("Enter name to show to other users.");
  promptInputbox->setText("");
  promptSubmit->removeEvent(CEGUI::PushButton::EventClicked);
  promptSubmit->subscribeEvent(CEGUI::PushButton::EventClicked,
                               CEGUI::Event::Subscriber(&MenuActivity::JoinServer,this));
}

bool MenuActivity::CancelPrompt( const CEGUI::EventArgs& e ) {
  CEGUI::System::getSingleton().getGUISheet()->removeChildWindow(promptWindow);
}

/*
  ====================================================
  = Player Select Menu
  ====================================================
*/
bool MenuActivity::SwitchToPlayerSelectMenu(const CEGUI::EventArgs& e){
  CEGUI::System::getSingleton().setGUISheet(app->Wmgr->getWindow("Menu/PlayerSelect"));

  CEGUI::Window* penguinButton = app->Wmgr->getWindow("Menu/Penguin");
  CEGUI::Window* ogreButton = app->Wmgr->getWindow("Menu/Ogre");

  penguinButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                                CEGUI::Event::Subscriber(&MenuActivity::SelectPenguin, this));

  ogreButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&MenuActivity::SelectOgre, this));
}

bool MenuActivity::SelectPenguin( const CEGUI::EventArgs& e){
  player_flag = 0;
  MenuActivity::SinglePlayerLevelSelectWrapper(e);

}
bool MenuActivity::SelectOgre( const CEGUI::EventArgs& e){
  player_flag = 1;
  MenuActivity::SinglePlayerLevelSelectWrapper(e);

}

/*
  ==========================================================
  = Level Selection Menu
  ==========================================================
*/

bool MenuActivity::SwitchToLevelSelectMenu( const CEGUI::EventArgs& e ) {
  close();
  if (CEGUI::System::getSingleton().getGUISheet()->getName().compare("Leaderboard") == 0) {
    CEGUI::System::getSingleton().setGUISheet(levelSelectorWindow);
    return true;
  }

  CEGUI::System::getSingleton().setGUISheet(levelSelectorWindow);
  LevelLoader *loader = LevelLoader::getSingleton();

  // This variable ensures that viewers are only initiated once, then reused.
  static bool initViewers = false;

  // Load each level in level viewer
  int selectorEnd = selectorStart + 8; //(selectorRows*selectorColumns);

  for (int i = 0; i < 8; i++) {
    lsButtons[i]->setVisible(false);
    if (viewerPool.size() > i) {
      viewerPool[i]->window->setVisible(false);
    }
  }

  for (int i = selectorStart; i < selectorEnd && i < loader->levelNames.size(); i++) {
    LevelViewer *v;

    if (initViewers) {
      // Recycle your Level Viewers!
      if (viewerPool.empty()) break;
      v = viewerPool.back();
      viewerPool.pop_back();
      v->loadLevel(loader->levelNames[i].c_str());
      v->window->setVisible(true);
    } else {
      v = new LevelViewer(app->mRenderer, loader->levelNames[i].c_str());
    }

    levelSelectorWindow->addChildWindow(v->window);

    if(type_flag == SINGLE_PLAYER){
      v->window->removeEvent(CEGUI::PushButton::EventMouseClick);
      v->window->subscribeEvent(CEGUI::PushButton::EventMouseClick,
                                CEGUI::Event::Subscriber(&MenuActivity::StartSinglePlayer, this));

      lsButtons[i % 8]->setVisible(true);
      lsButtons[i % 8]->removeEvent(CEGUI::PushButton::EventMouseClick);
      lsButtons[i % 8]->subscribeEvent(CEGUI::PushButton::EventMouseClick,
                                       CEGUI::Event::Subscriber(&MenuActivity::ShowLeaderboard, this));
    }
    else if (type_flag == MULTI_HOST){
      v->window->removeEvent(CEGUI::PushButton::EventMouseClick);
      v->window->subscribeEvent(CEGUI::PushButton::EventMouseClick,
                                CEGUI::Event::Subscriber(&MenuActivity::StartMultiPlayerHost, this));
    }
    v->setPositionPercent(0.05 + ((i%8)%selectorColumns)*0.9/selectorColumns,
                          0.2 + ((i%8)/selectorColumns)*0.6/selectorRows);

    levelViewers.push_back(v);
  }

  initViewers = true;
}

bool MenuActivity::handleLSPrev( const CEGUI::EventArgs& evt ) {
  if (selectorStart > 0) {
    selectorStart -= 8;
    SwitchToLevelSelectMenu(evt);
  }
}

bool MenuActivity::handleLSNext( const CEGUI::EventArgs& evt ) {
  if (selectorStart < app->levelLoader->levelNames.size() - 8) {
    selectorStart += 8;
    SwitchToLevelSelectMenu(evt);
  }
}

bool MenuActivity::ShowLeaderboard( const CEGUI::EventArgs& e ) {
  CEGUI::Window* leaderboardWindow, *leaderboardName, *leaderboardNextLevel, *leaderboardBackToMenu;
  CEGUI::Window* leaderboardWindows[10];

  leaderboardWindow = app->Wmgr->getWindow("Leaderboard");
  leaderboardName = app->Wmgr->getWindow("Leaderboard/LevelName");
  leaderboardNextLevel = app->Wmgr->getWindow("Leaderboard/NextLevel");
  leaderboardBackToMenu = app->Wmgr->getWindow("Leaderboard/BackToMenu");

  for (int i = 0; i < 10; i++) {
    std::stringstream ss;
    ss << "Leaderboard/" << i;
    leaderboardWindows[i] = app->Wmgr->getWindow(ss.str());
  }

  CEGUI::System::getSingleton().setGUISheet(leaderboardWindow);

  leaderboardBackToMenu->removeEvent(CEGUI::PushButton::EventClicked);
  leaderboardBackToMenu
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
                     CEGUI::Event::Subscriber(&MenuActivity::SwitchToLevelSelectMenu, this));

  int levelViewerIndex =
    *static_cast<const CEGUI::MouseEventArgs*>(&e)->window->getName().rbegin() - '1';

  CEGUI::String name = levelViewers[levelViewerIndex]->window->getName();

  leaderboardNextLevel->setVisible(false);
  /*
    leaderboardNextLevel->removeEvent(CEGUI::PushButton::EventClicked);
    leaderboardNextLevel
    ->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MenuActivity::awdawd, this));
  */

  for (int i = 0; i < 10; i++)
    leaderboardWindows[i]->setAlpha(0.0);

  Leaderboard leaderboard = Leaderboard::findLeaderboard(name.c_str());
  leaderboardName->setText(name.c_str());
  OBAnimationManager::startAnimation("SpinPopup", leaderboardName);
  OBAnimationManager::startAnimation("SpinPopup", leaderboardNextLevel, 0.5);
  OBAnimationManager::startAnimation("SpinPopup", leaderboardBackToMenu, 0.5);

  multimap<double, LeaderboardEntry, greater<double> > highscores = leaderboard.getHighscores();
  multimap<double, LeaderboardEntry>::iterator iter;

  int i = 0;
  for (iter = highscores.begin(); iter != highscores.end(); iter++) {
    LeaderboardEntry entry = iter->second;
    std::stringstream ss;

    size_t namelen = entry.name.length();
    ss << std::left << setw(55-namelen) << entry.name <<
      setw(15) << entry.score <<
      setw(15) << entry.getTimeTaken() <<
      setw(25) << entry.getTimeEntered();

    leaderboardWindows[i]->setText(ss.str());

    OBAnimationManager::startAnimation("FadeInFromLeft", leaderboardWindows[i], 1.0, 1.0 + 0.2f*i);
    i++;
  }
}

bool MenuActivity::SinglePlayerLevelSelectWrapper( const CEGUI::EventArgs& e ){
  type_flag = SINGLE_PLAYER;
  MenuActivity::SwitchToLevelSelectMenu(e);
}

bool MenuActivity::MultiPlayerLevelSelectWrapper( const CEGUI::EventArgs& e ){
  type_flag = MULTI_HOST;
  MenuActivity::SwitchToLevelSelectMenu(e);
}

/*
  ==========================================================
  = Transition Methods
  ==========================================================
*/

bool MenuActivity::StartSinglePlayer( const CEGUI::EventArgs& e ) {
  CEGUI::MouseCursor::getSingleton().hide();

  CEGUI::String levelName = static_cast<const CEGUI::MouseEventArgs*>(&e)->window->getName();
  LevelLoader *loader = LevelLoader::getSingleton();
  for(int i = 0; i < loader->levelNames.size(); i++)
    {

    }
  app->switchActivity(new SinglePlayerActivity(app, levelName.c_str(), player_flag));
  return true;
}

bool MenuActivity::StartMultiPlayerHost( const CEGUI::EventArgs& e ){
  CEGUI::MouseCursor::getSingleton().hide();

  CEGUI::String levelName = static_cast<const CEGUI::MouseEventArgs*>(&e)->window->getName();
  app->switchActivity(new HostPlayerActivity(app, levelName.c_str()));
  return true;

}

bool MenuActivity::StartMultiPlayerClient( const CEGUI::EventArgs& e) {
/*  CEGUI::MouseCursor::getSingleton().hide();
  CEGUI::String hostName = static_cast<const CEGUI::MouseEventArgs*>(&e)->window->getText();
  printf("host name is %s\n", hostName.c_str());
  app->switchActivity(new ClientPlayerActivity(app));*/
  return true;
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
