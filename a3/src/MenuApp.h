#include "BaseApplication.h"
 
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
 
static const int MODE_MENU = 0;
static const int MODE_SP = 1;
static const int MODE_MP_HOST = 2;
static const int MODE_MP_CLIENT = 3;
static const int MODE_QUIT = 4;
static int mode = MODE_MENU;

class MenuApp : public BaseApplication
{
 public:
  MenuApp(void);
  virtual ~MenuApp(void);
 
 protected:
  virtual void createScene(void);
  virtual void createFrameListener(void);
 
  // Ogre::FrameListener
  virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
 
  // OIS::KeyListener
  virtual bool keyPressed( const OIS::KeyEvent &arg );
  virtual bool keyReleased( const OIS::KeyEvent &arg );
  // OIS::MouseListener
  virtual bool mouseMoved( const OIS::MouseEvent &arg );
  virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
  virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

  bool SwitchToMainMenu(const CEGUI::EventArgs& e);
  bool SwitchToMultiMenu(const CEGUI::EventArgs& e);
  bool StartSinglePlayer(const CEGUI::EventArgs& e);
  bool StartHost(const CEGUI::EventArgs& e);
  bool StartClient(const CEGUI::EventArgs& e);
  bool quit(const CEGUI::EventArgs &e);
};
