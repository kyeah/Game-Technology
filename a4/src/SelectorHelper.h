#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>

#include "LevelViewer.h"

class SelectorHelper {
 public:
  static const int TYPE_NOT_SELECTED  = -1;
  static const int TYPE_SINGLE_PLAYER = 0;
  static const int TYPE_MULTI_HOST    = 1;
  static const int TYPE_MULTI_CHANGE  = 2;
  static const int TYPE_MULTI_CLIENT  = 3;

  static const int CHARACTER_PENGUIN = 0;
  static const int CHARACTER_OGRE = 1;
  static const int CHARACTER_NINJA = 2;

  static const int selectorRows = 2;
  static const int selectorColumns = 4;

  static std::vector<LevelViewer*> viewerPool;
  static std::vector<LevelViewer*> levelViewers;

  static int selectorStart;
  static int type_flag, player_flag;

  static void SwitchToLevelSelectMenu(void);
  static void SwitchToPlayerSelectMenu(void);

  static void recycleViewers( void );
  static bool handleLSPrev( const CEGUI::EventArgs &e );
  static bool handleLSNext( const CEGUI::EventArgs &e );
  static bool SwitchToLevelSelectMenu( const CEGUI::EventArgs &e );

  static void frameStarted( float elapsedTime );

  static bool SelectCharacter( const CEGUI::EventArgs &e );
  static bool ShowLeaderboard( const CEGUI::EventArgs &e );
  static char* CharacterToString(int character);
};
