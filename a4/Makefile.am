ACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= libs/ConfigLoader.hpp libs/MeshStrider.h src/LevelLoader.h src/BaseApplication.h src/OgreBallApplication.h src/Physics.h src/OgreMotionState.h src/GameObject.h src/Collisions.h src/Activity.h src/MenuActivity.h src/SinglePlayerActivity.h src/Interpolator.h src/LevelViewer.h src/CameraObject.h src/Sounds.h src/ClientPlayerActivity.h src/HostPlayerActivity.h src/Networking.h src/common.h src/Leaderboard.h src/OBAnimationManager.h

bin_PROGRAMS= OgreBall 
OgreBall_CPPFLAGS= -I$(top_srcdir) -Ilibs/OgreProcedural/include -std=c++0x

OgreBall_SOURCES= libs/ConfigScript.cpp libs/MeshStrider.cpp src/BaseApplication.cpp src/OgreBallApplication.cpp src/Physics.cpp src/OgreMotionState.cpp src/GameObject.cpp src/OgreBall.cpp src/Plane.cpp src/LevelLoader.cpp src/MeshObject.cpp src/MenuActivity.cpp src/SinglePlayerActivity.cpp src/Interpolator.cpp src/Collisions.cpp src/GoalObject.cpp src/Collectible.cpp src/LevelViewer.cpp src/DecorativeObject.cpp src/CameraObject.cpp src/Sounds.cpp src/Collidable.cpp src/ClientPlayerActivity.cpp src/HostPlayerActivity.cpp src/Networking.cpp src/common.cpp

OgreBall_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS) $(bullet_CFLAGS) $(SDLMIXER_CFLAGS) $(sdl_CFLAGS) $(SDLNET_CFLAGS) $(CEGUI_CFLAGS) $(CEGUI_OGRE_CFLAGS) -lboost_system -lboost_filesystem -Llibs/OgreProcedural/lib -lOgreProcedural -Wl,-rpath=libs/OgreProcedural/lib -lboost_serialization
OgreBall_LDADD= $(OGRE_LIBS) $(OIS_LIBS) $(bullet_LIBS) $(SDLMIXER_LIBS) $(CEGUI_LIBS) $(CEGUI_OGRE_LIBS) $(sdl_LIBS) $(SDLNET_LIBS)

EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
