ACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= libs/ConfigLoader.hpp src/LevelLoader.h src/BaseApplication.h src/OgreBallApplication.h src/Physics.h src/OgreMotionState.h src/GameObject.h src/Collisions.h 

bin_PROGRAMS= OgreBall 
OgreBall_CPPFLAGS= -I$(top_srcdir) -std=c++0x
OgreBall_SOURCES= libs/ConfigScript.cpp src/BaseApplication.cpp src/OgreBallApplication.cpp src/Physics.cpp src/OgreMotionState.cpp src/GameObject.cpp src/OgreBall.cpp src/Plane.cpp src/LevelLoader.cpp
OgreBall_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) $(CEGUI_CFLAGS) $(CEGUI_OGRE_CFLAGS) -lboost_system -lboost_filesystem
OgreBall_LDADD= $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) $(CEGUI_LIBS) $(CEGUI_OGRE_LIBS)

EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
