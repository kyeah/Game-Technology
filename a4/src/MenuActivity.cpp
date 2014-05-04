#include <OgreOverlayManager.h>
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
#include "SelectorHelper.h"

#define NOT_SELECTED -1
#define SINGLE_PLAYER 0
#define MULTI_HOST 1
#define MULTI_CLIENT 2

MenuActivity::MenuActivity(OgreBallApplication *app) : Activity(app) {

}

MenuActivity::~MenuActivity(void) {
  close();
}

void MenuActivity::close(void) {
  SelectorHelper::recycleViewers();
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

  // Menu
  mainMenuSheet = app->Wmgr->getWindow("Menu/Background");

  singlePlayerButton = app->Wmgr->getWindow("Menu/SinglePlayer");
  multiPlayerButton = app->Wmgr->getWindow("Menu/MultiPlayer");
  quitButton = app->Wmgr->getWindow("Menu/QuitGame");

  singlePlayerButton->removeEvent(CEGUI::PushButton::EventClicked);
  singlePlayerButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                                     CEGUI::Event::Subscriber(&MenuActivity::SwitchToPlayerSelectMenu, this));

  multiPlayerButton->removeEvent(CEGUI::PushButton::EventClicked);
  multiPlayerButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                                    CEGUI::Event::Subscriber(&MenuActivity::SwitchToMultiMenu, this));

  quitButton->removeEvent(CEGUI::PushButton::EventClicked);
  quitButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&MenuActivity::quit,this));

  // Multiplayer Base Menu
  multiMenuSheet = app->Wmgr->getWindow("Menu/MultiBackground");

  hostButton = app->Wmgr->getWindow("Menu/Host");
  clientButton = app->Wmgr->getWindow("Menu/Client");
  returnButton = app->Wmgr->getWindow("Menu/Return");

  hostButton->removeEvent(CEGUI::PushButton::EventClicked);
  hostButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&MenuActivity::PromptForHost,this));

  clientButton->removeEvent(CEGUI::PushButton::EventClicked);
  clientButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                               CEGUI::Event::Subscriber(&MenuActivity::SwitchToServerListMenu,this));

  returnButton->removeEvent(CEGUI::PushButton::EventClicked);
  returnButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                               CEGUI::Event::Subscriber(&MenuActivity::SwitchToMainMenu, this));

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

  // Host Prompt
  hPromptWindow = wmgr->getWindow("HostPrompt");
  hPromptHeader = wmgr->getWindow("HostPrompt/Header");
  hPromptLobbyNameInputbox = (CEGUI::Editbox*)wmgr->getWindow("HostPrompt/LobbyNameInput");
  hPromptInputbox = (CEGUI::Editbox*)wmgr->getWindow("HostPrompt/Input");
  hPromptSubmit = wmgr->getWindow("HostPrompt/Submit");
  hPromptCancel = wmgr->getWindow("HostPrompt/Cancel");

  hPromptCancel->subscribeEvent(CEGUI::PushButton::EventClicked,
                                CEGUI::Event::Subscriber(&MenuActivity::CancelHprompt, this));

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
  app->mPhysics->stepSimulation(elapsedTime);

  SelectorHelper::frameStarted(elapsedTime);

  return true;
}

/*
  ==========================================================
  = Main Menu
  ==========================================================
*/

bool MenuActivity::SwitchToMainMenu( const CEGUI::EventArgs& e ) {
  SelectorHelper::selectorStart = 0;
  close();

  SelectorHelper::type_flag = NOT_SELECTED;
  CEGUI::System::getSingleton().setGUISheet(mainMenuSheet);
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
  CEGUI::System::getSingleton().setGUISheet(multiMenuSheet);
}

bool MenuActivity::SwitchToServerListMenu( const CEGUI::EventArgs& e ) {
  CEGUI::System::getSingleton().setGUISheet(serverListWindow);

  // Ping all known hosts from file
  std::vector<PingResponseMessage*> responses = Networking::hostCheck( "data/hosts/hosts.txt" );

  // Display list of active hosts waiting to start a game
  serverListbox->resetList();

  for (int i = 0; i < responses.size(); i++) {
    std::stringstream ss;
    ss << responses[i]->lobbyName << " (" << responses[i]->numPlayers << "/" << responses[i]->maxPlayers << ")";
    CEGUI::ListboxTextItem* chatItem = new CEGUI::ListboxTextItem(ss.str());
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

bool MenuActivity::PromptForHost( const CEGUI::EventArgs& e ) {
  CEGUI::System::getSingleton().getGUISheet()->addChildWindow(hPromptWindow);
  hPromptHeader->setText("Enter your lobby name and username.");
  hPromptLobbyNameInputbox->setText("");
  hPromptInputbox->setText("");
  hPromptSubmit->removeEvent(CEGUI::PushButton::EventClicked);
  hPromptSubmit->subscribeEvent(CEGUI::PushButton::EventClicked,
                                CEGUI::Event::Subscriber(&MenuActivity::MultiPlayerLevelSelectWrapper, this));
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

bool MenuActivity::CancelHprompt( const CEGUI::EventArgs& e ) {
  CEGUI::System::getSingleton().getGUISheet()->removeChildWindow(hPromptWindow);
}

/*
  ====================================================
  = Player Select Menu
  ====================================================
*/
bool MenuActivity::SwitchToPlayerSelectMenu(const CEGUI::EventArgs& e){
  SelectorHelper::type_flag = SelectorHelper::TYPE_SINGLE_PLAYER;
  SelectorHelper::SwitchToPlayerSelectMenu();
}

/*
  ==========================================================
  = Level Selection Menu
  ==========================================================
*/

bool MenuActivity::SinglePlayerLevelSelectWrapper( const CEGUI::EventArgs& e ){
  SelectorHelper::type_flag = SelectorHelper::TYPE_SINGLE_PLAYER;
  SelectorHelper::SwitchToLevelSelectMenu();
}

bool MenuActivity::MultiPlayerLevelSelectWrapper( const CEGUI::EventArgs& e ){
  SelectorHelper::type_flag = SelectorHelper::TYPE_MULTI_HOST;
  SelectorHelper::SwitchToLevelSelectMenu();
}

/*
  ==========================================================
  = Transition Methods
  ==========================================================
*/

bool MenuActivity::StartSinglePlayer( const CEGUI::EventArgs& e ) {
  CEGUI::MouseCursor::getSingleton().hide();

  CEGUI::String levelName = static_cast<const CEGUI::MouseEventArgs*>(&e)->window->getName();
  app->switchActivity(new SinglePlayerActivity(app, levelName.c_str(), SelectorHelper::player_flag));
  return true;
}

bool MenuActivity::StartMultiPlayerHost( const CEGUI::EventArgs& e ){
  SelectorHelper::recycleViewers();
  CEGUI::String lobbyname = hPromptLobbyNameInputbox->getText();
  CEGUI::String name = hPromptInputbox->getText();
  if (!lobbyname.length() || !name.length()) return true;

  CEGUI::System::getSingleton().getGUISheet()->removeChildWindow(hPromptWindow);

  CEGUI::String levelName = static_cast<const CEGUI::MouseEventArgs*>(&e)->window->getName();
  app->switchActivity(new HostPlayerActivity(app, lobbyname.c_str(), name.c_str(), levelName.c_str()));
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
