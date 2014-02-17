ACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= src/BaseApplication.h src/RacquetApp.h src/Physics.h src/OgreMotionState.h src/GameObject.h src/RacquetObject.h src/Collisions.h

bin_PROGRAMS= RacquetApp
RacquetApp_CPPFLAGS= -I$(top_srcdir) -I/usr/include/bullet -I/usr/local/include/bullet -std=c++11
RacquetApp_SOURCES= src/BaseApplication.cpp src/RacquetApp.cpp src/Physics.cpp src/OgreMotionState.cpp src/GameObject.cpp src/Ball.cpp src/Racquet.cpp
RacquetApp_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS)
RacquetApp_LDADD= $(OGRE_LIBS) $(OIS_LIBS) -lboost_system -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
