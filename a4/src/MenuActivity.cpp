#include "MenuActivity.h"

void MenuActivity::start(void) {
  const CEGUI::EventArgs* args;
  SwitchToMainMenu(*args);
}

bool MenuActivity::frameRenderingQueued( const Ogre::FrameEvent& evt ) {
  CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);
  return true;
}

bool MenuActivity::frameStarted( const Ogre::FrameEvent& evt ) {
  
}

bool MenuActivity::SwitchToMainMenu( const CEGUI::EventArgs& e ) {
  /*    menu = Wmgr->getWindow("Menu/Background");
    CEGUI::System::getSingleton().setGUISheet(menu);

    CEGUI::PushButton* singlePlayerButton = (CEGUI::PushButton*)Wmgr->getWindow("Menu/SinglePlayer");
    CEGUI::PushButton* multiPlayerButton = (CEGUI::PushButton*)Wmgr->getWindow("Menu/MultiPlayer");
    CEGUI::PushButton* quitButton = (CEGUI::PushButton*)Wmgr->getWindow("Menu/QuitGame");

    singlePlayerButton->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MenuActivity::StartSinglePlayer, this));
    multiPlayerButton->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MenuActivity::SwitchToMultiMenu, this));
    quitButton->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MenuActivity::quit,this));*/
}

bool MenuActivity::SwitchToMultiMenu( const CEGUI::EventArgs& e ) {
  /*  menu = Wmgr->getWindow("Menu/MultiBackground");
  CEGUI::System::getSingleton().setGUISheet(menu);
  
  CEGUI::PushButton* hostButton = (CEGUI::PushButton*)Wmgr->getWindow("Menu/Host");
  CEGUI::PushButton* clientButton = (CEGUI::PushButton*)Wmgr->getWindow("Menu/Client");
  CEGUI::PushButton* returnButton = (CEGUI::PushButton*)Wmgr->getWindow("Menu/Return");
  
  hostButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                             CEGUI::Event::Subscriber(&MenuActivity::StartHost,this));
  clientButton->subscribeEvent(CEGUI::PushButton::EventClicked,
                               CEGUI::Event::Subscriber(&MenuActivity::StartClient,this));
  returnButton->subscribeEvent(CEGUI::PushButton::EventClicked,
  CEGUI::Event::Subscriber(&MenuActivity::SwitchToMainMenu, this));*/
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

CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID)
{
  switch (buttonID)
    {
    case OIS::MB_Left:
      return CEGUI::LeftButton;

    case OIS::MB_Right:
      return CEGUI::RightButton;

    case OIS::MB_Middle:
      return CEGUI::MiddleButton;

    default:
      return CEGUI::LeftButton;
    }
}

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
  CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(id));
  return true;
}

//-------------------------------------------------------------------------------------                          

bool MenuActivity::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )                                 
{                                                                                                                
  CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(id));                                          
  return true;                                                                                                   
}                                                                                                                
  
