ACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= src/BaseApplication.h src/RacquetApp.h src/Physics.h src/OgreMotionState.h src/GameObject.h src/RacquetObject.h src/Collisions.h

bin_PROGRAMS= RacquetApp
RacquetApp_CPPFLAGS= -I$(top_srcdir) -std=c++0x
RacquetApp_SOURCES= src/BaseApplication.cpp src/RacquetApp.cpp src/Physics.cpp src/OgreMotionState.cpp src/GameObject.cpp src/Ball.cpp src/Racquet.cpp src/Plane.cpp
RacquetApp_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) $(sdl_CFLAGS) $(SDLMIXER_CFLAGS) -lboost_system
RacquetApp_LDADD= $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) $(sdl_LIBS) $(SDLMIXER_LIBS)
EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
