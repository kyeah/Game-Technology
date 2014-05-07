#include "SelectorHelper.h"

#include "LevelLoader.h"
#include "OgreBallApplication.h"
#include "MenuActivity.h"
#include "HostPlayerActivity.h"
#include "ClientPlayerActivity.h"
#include "Leaderboard.h"
#include "OBAnimationManager.h"

std::vector<LevelViewer*> SelectorHelper::viewerPool;
std::vector<LevelViewer*> SelectorHelper::levelViewers;

int SelectorHelper::selectorStart = 0;
int SelectorHelper::type_flag = SelectorHelper::TYPE_NOT_SELECTED;
int SelectorHelper::player_flag = SelectorHelper::CHARACTER_PENGUIN;

void SelectorHelper::recycleViewers(void) {
  for (int i = 0; i < levelViewers.size(); i++) {
    viewerPool.push_back(levelViewers[i]);
  }

  levelViewers.clear();
}

bool SelectorHelper::handleLSPrev( const CEGUI::EventArgs& e ) {
  if (selectorStart > 0) {
    selectorStart -= 8;
    SwitchToLevelSelectMenu();
  }
}

bool SelectorHelper::handleLSNext( const CEGUI::EventArgs& evt ) {
  if (selectorStart < LevelLoader::getSingleton()->levelNames.size() - 8) {
    selectorStart += 8;
    SwitchToLevelSelectMenu();
  }
}

void SelectorHelper::frameStarted( float elapsedTime ) {
  for (int i = 0; i < levelViewers.size(); i++) {
    levelViewers[i]->frameStarted(elapsedTime);
  }
}

char* SelectorHelper::CharacterToString( int character ) {
  switch(character){
  case SelectorHelper::CHARACTER_PENGUIN:
    return "penguin.mesh";
  case SelectorHelper::CHARACTER_OGRE:
    return "ogrehead.mesh";
  case SelectorHelper::CHARACTER_NINJA:
    return "ninja.mesh";
  default:
    return "penguin.mesh";
  }
}

/*
  ==============================
  = Level Selector Switch
  ==============================
*/


bool SelectorHelper::SwitchToLevelSelectMenu( const CEGUI::EventArgs &e ) {
  SwitchToLevelSelectMenu();
}

void SelectorHelper::SwitchToLevelSelectMenu(void) {
  recycleViewers();

  CEGUI::WindowManager *wmgr = CEGUI::WindowManager::getSingletonPtr();
  CEGUI::Window *levelSelectorWindow = wmgr->getWindow("Menu/LevelSelector");

  if (CEGUI::System::getSingleton().getGUISheet()->getName().compare("Leaderboard") == 0) {
    CEGUI::System::getSingleton().setGUISheet(levelSelectorWindow);
    return;
  }

  // Pre-load windows
  LevelLoader *loader = LevelLoader::getSingleton();
  CEGUI::Window *lsBack = wmgr->getWindow("LevelSelector/Back");
  CEGUI::Window *lsPrev = wmgr->getWindow("LevelSelector/Prev");
  CEGUI::Window *lsNext = wmgr->getWindow("LevelSelector/Next");
  CEGUI::Window *lsButtons[8];

  for (int i = 0; i < 8; i++) {
    std::stringstream ss;
    ss << "LevelSelector/" << i+1;
    lsButtons[i] = wmgr->getWindow(ss.str());
  }

  lsPrev->removeEvent(CEGUI::PushButton::EventClicked);
  lsPrev->subscribeEvent(CEGUI::PushButton::EventClicked, &SelectorHelper::handleLSPrev);

  lsNext->removeEvent(CEGUI::PushButton::EventClicked);
  lsNext->subscribeEvent(CEGUI::PushButton::EventClicked, &SelectorHelper::handleLSNext);

  CEGUI::System::getSingleton().setGUISheet(levelSelectorWindow);

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
      v = new LevelViewer(OgreBallApplication::getSingleton()->mRenderer, loader->levelNames[i].c_str());

    }

    levelSelectorWindow->addChildWindow(v->window);
    if(type_flag == TYPE_SINGLE_PLAYER || type_flag == TYPE_MULTI_HOST){
      MenuActivity *activity = (MenuActivity*) OgreBallApplication::getSingleton()->activity;

      lsBack->removeEvent(CEGUI::PushButton::EventClicked);
      lsBack->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&MenuActivity::SwitchToMainMenu, activity));

      if (type_flag == TYPE_SINGLE_PLAYER) {
        v->window->removeEvent(CEGUI::PushButton::EventMouseClick);
        v->window->subscribeEvent(CEGUI::PushButton::EventMouseClick,
                                  CEGUI::Event::Subscriber(&MenuActivity::StartSinglePlayer, activity));

        lsButtons[i % 8]->setVisible(true);
        lsButtons[i % 8]->removeEvent(CEGUI::PushButton::EventMouseClick);
        lsButtons[i % 8]->subscribeEvent(CEGUI::PushButton::EventMouseClick,
                                         &SelectorHelper::ShowLeaderboard);
      } else {
        v->window->removeEvent(CEGUI::PushButton::EventMouseClick);
        v->window->subscribeEvent(CEGUI::PushButton::EventMouseClick,
                                  CEGUI::Event::Subscriber(&MenuActivity::StartMultiPlayerHost, activity));
      }

    } else if (type_flag == TYPE_MULTI_CHANGE) {
      HostPlayerActivity *activity = (HostPlayerActivity*) OgreBallApplication::getSingleton()->activity;
      v->window->removeEvent(CEGUI::PushButton::EventMouseClick);
      v->window->subscribeEvent(CEGUI::PushButton::EventMouseClick,
                                CEGUI::Event::Subscriber(&HostPlayerActivity::handleLevelSelected, activity));
    }

    v->setPositionPercent(0.05 + ((i%8)%selectorColumns)*0.9/selectorColumns,
                          0.2 + ((i%8)/selectorColumns)*0.6/selectorRows);

    levelViewers.push_back(v);
  }

  initViewers = true;
}

/*
  ==============================
  = Player Selector Switch
  ==============================
*/


void SelectorHelper::SwitchToPlayerSelectMenu(void) {
  CEGUI::WindowManager *wmgr = CEGUI::WindowManager::getSingletonPtr();

  CEGUI::System::getSingleton().setGUISheet(wmgr->getWindow("Menu/PlayerSelect"));
  CEGUI::Window* penguinButton = wmgr->getWindow("Menu/Penguin");
  CEGUI::Window* ogreButton = wmgr->getWindow("Menu/Ogre");
  CEGUI::Window* ninjaButton = wmgr->getWindow("Menu/Ninja");

  penguinButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                                &SelectorHelper::SelectCharacter);

  ogreButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                             &SelectorHelper::SelectCharacter);

  ninjaButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                              &SelectorHelper::SelectCharacter);
}

bool SelectorHelper::SelectCharacter( const CEGUI::EventArgs& e ) {
  CEGUI::Window* window = static_cast<const CEGUI::MouseEventArgs*>(&e)->window;

  CEGUI::WindowManager *wmgr = CEGUI::WindowManager::getSingletonPtr();
  CEGUI::Window* penguinButton = wmgr->getWindow("Menu/Penguin");
  CEGUI::Window* ogreButton = wmgr->getWindow("Menu/Ogre");
  CEGUI::Window* ninjaButton = wmgr->getWindow("Menu/Ninja");

  if (window == penguinButton) player_flag = CHARACTER_PENGUIN;
  else if (window == ogreButton) player_flag = CHARACTER_OGRE;
  else if (window == ninjaButton) player_flag = CHARACTER_NINJA;

  if (type_flag == TYPE_SINGLE_PLAYER) {
    MenuActivity *activity = (MenuActivity*) OgreBallApplication::getSingleton()->activity;
    activity->SinglePlayerLevelSelectWrapper(e);

  } else if (type_flag == TYPE_MULTI_HOST) {
    HostPlayerActivity *activity = (HostPlayerActivity*) OgreBallApplication::getSingleton()->activity;
    activity->handlePlayerSelected(player_flag);

  } else if (type_flag == TYPE_MULTI_CLIENT) {
    ClientPlayerActivity *activity = (ClientPlayerActivity*) OgreBallApplication::getSingleton()->activity;
    activity->handlePlayerSelected(player_flag);
  }
}

bool SelectorHelper::ShowLeaderboard( const CEGUI::EventArgs &e ) {
  CEGUI::WindowManager *wmgr = CEGUI::WindowManager::getSingletonPtr();
  CEGUI::Window* leaderboardWindow, *leaderboardName, *leaderboardNextLevel, *leaderboardBackToMenu;
  CEGUI::Window* leaderboardWindows[10];

  leaderboardWindow = wmgr->getWindow("Leaderboard");
  leaderboardName = wmgr->getWindow("Leaderboard/LevelName");
  leaderboardNextLevel = wmgr->getWindow("Leaderboard/NextLevel");
  leaderboardBackToMenu = wmgr->getWindow("Leaderboard/BackToMenu");

  for (int i = 0; i < 10; i++) {
    std::stringstream ss;
    ss << "Leaderboard/" << i;
    leaderboardWindows[i] = wmgr->getWindow(ss.str());
  }

  CEGUI::System::getSingleton().setGUISheet(leaderboardWindow);

  leaderboardBackToMenu->removeEvent(CEGUI::PushButton::EventClicked);
  leaderboardBackToMenu->subscribeEvent(CEGUI::PushButton::EventClicked,
                                        &SelectorHelper::SwitchToLevelSelectMenu);

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

  std::multimap<double, LeaderboardEntry, greater<double> > highscores = leaderboard.getHighscores();
  std::multimap<double, LeaderboardEntry>::iterator iter;

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
