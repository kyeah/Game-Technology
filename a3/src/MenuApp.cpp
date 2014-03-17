#include "MenuApp.h"

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>

//-------------------------------------------------------------------------------------
MenuApp::MenuApp(void)
{
}
//-------------------------------------------------------------------------------------
MenuApp::~MenuApp(void)
{
}
//-------------------------------------------------------------------------------------
void MenuApp::createScene(void)
{
  // Resources
  CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
  CEGUI::Font::setDefaultResourceGroup("Fonts");
  CEGUI::Scheme::setDefaultResourceGroup("Schemes");
  CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
  CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

  mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
  // CEGUI::Logger::getSingleton().setLoggingLevel(CEGUI::Informative);
  CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
  CEGUI::SchemeManager::getSingleton().create("WindowsLook.scheme");
  CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook", "MouseArrow");
  CEGUI::WindowManager* Wmgr = CEGUI::WindowManager::getSingletonPtr();

  try
    {
      CEGUI::Window* menu = Wmgr->loadWindowLayout("Menu.layout");
      CEGUI::System::getSingleton().setGUISheet(menu);
      //      myRoot->addChildWindow(menu);
    }
  catch(CEGUI::Exception &e)
    {
      OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, std::string(e.getMessage().c_str()), "Error Parsing Menu");
    }
}
//-------------------------------------------------------------------------------------
void MenuApp::createFrameListener(void)
{
  Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
  OIS::ParamList pl;
  size_t windowHnd = 0;
  std::ostringstream windowHndStr;

  mWindow->getCustomAttribute("WINDOW", &windowHnd);
  windowHndStr << windowHnd;
  pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

  mInputManager = OIS::InputManager::createInputSystem( pl );

  mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
  mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

  mMouse->setEventCallback(this);
  mKeyboard->setEventCallback(this);

  //Set initial mouse clipping size
  windowResized(mWindow);

  //Register as a Window listener
  Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

  mRoot->addFrameListener(this);
  //  BaseApplication::createFrameListener();
}
//-------------------------------------------------------------------------------------
bool MenuApp::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
  if(mWindow->isClosed())
    return false;

  if(mShutDown)
    return false;

  //Need to capture/update each device
  mKeyboard->capture();
  mMouse->capture();

  //Need to inject timestamps to CEGUI System.
  CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);

  return true;
  // return BaseApplication::frameRenderingQueued(evt);
}
//-------------------------------------------------------------------------------------
bool MenuApp::keyPressed( const OIS::KeyEvent &arg )
{
  CEGUI::System &sys = CEGUI::System::getSingleton();
  sys.injectKeyDown(arg.key);
  sys.injectChar(arg.text);
  return true;
  //  return BaseApplication::keyPressed(arg);
}
//-------------------------------------------------------------------------------------
bool MenuApp::keyReleased( const OIS::KeyEvent &arg )
{
  CEGUI::System::getSingleton().injectKeyUp(arg.key);
  return true;
  //  return BaseApplication::keyReleased(arg);
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

bool MenuApp::mouseMoved( const OIS::MouseEvent &arg )
{
  CEGUI::System &sys = CEGUI::System::getSingleton();
  sys.injectMouseMove(arg.state.X.rel, arg.state.Y.rel);
  // Scroll wheel.
  if (arg.state.Z.rel)
    sys.injectMouseWheelChange(arg.state.Z.rel / 120.0f);
  return true;
  // return BaseApplication::mouseMoved(arg);
}
//-------------------------------------------------------------------------------------
bool MenuApp::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(id));
  return true;
  //  return BaseApplication::mousePressed(arg, id);
}
//-------------------------------------------------------------------------------------
bool MenuApp::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(id));
  return true;
  //  return BaseApplication::mouseReleased(arg, id);
}
//-------------------------------------------------------------------------------------
bool MenuApp::quit(const CEGUI::EventArgs &e)
{
  return true;
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
    MenuApp app;
    //    instance = &app;

    try {
      app.go();
    } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
      MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TA\
                  SKMODAL);
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
