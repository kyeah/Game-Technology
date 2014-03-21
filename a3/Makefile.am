ACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= src/BaseApplication.h src/MenuApp.h src/RacquetApp.h src/Physics.h src/OgreMotionState.h src/GameObject.h src/RacquetObject.h src/Collisions.h src/Sounds.h src/ClientApp.h src/Networking.h src/HostApp.h src/common.h src/BaseMultiplayerApp.h

bin_PROGRAMS= MenuApp
MenuApp_CPPFLAGS= -I$(top_srcdir) -std=c++0x
MenuApp_SOURCES= src/BaseApplication.cpp src/MenuApp.cpp src/RacquetApp.cpp src/Physics.cpp src/OgreMotionState.cpp src/GameObject.cpp src/Ball.cpp src/Racquet.cpp src/Plane.cpp src/Sounds.cpp src/Dude.cpp src/HostApp.cpp src/common.cpp src/BaseMultiplayerApp.cpp src/ClientApp.cpp
MenuApp_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) $(sdl_CFLAGS) $(SDLMIXER_CFLAGS) $(SDLNET_CFLAGS) $(CEGUI_CFLAGS) $(CEGUI_OGRE_CFLAGS) -lboost_system
MenuApp_LDADD= $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) $(sdl_LIBS) $(SDLMIXER_LIBS) $(SDLNET_LIBS) $(CEGUI_LIBS) $(CEGUI_OGRE_LIBS)
EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
