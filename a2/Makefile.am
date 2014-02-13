ACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= BaseApplication.h RacquetApp.h Physics.h OgreMotionState.h GameObject.h RacquetObject.h

bin_PROGRAMS= RacquetApp
RacquetApp_CPPFLAGS= -I$(top_srcdir) -I/usr/include/bullet -I/usr/local/include/bullet
RacquetApp_SOURCES= BaseApplication.cpp RacquetApp.cpp Physics.cpp OgreMotionState.cpp GameObject.cpp Ball.cpp Racquet.cpp
RacquetApp_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS)
RacquetApp_LDADD= $(OGRE_LIBS) $(OIS_LIBS) -lboost_system -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
