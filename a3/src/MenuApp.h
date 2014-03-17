#include "BaseApplication.h"
 
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
 
class MenuApp : public BaseApplication
{
 public:
  MenuApp(void);
  virtual ~MenuApp(void);
 
 protected:
  CEGUI::OgreRenderer* mRenderer;
 
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
  
  bool quit(const CEGUI::EventArgs &e);
};
