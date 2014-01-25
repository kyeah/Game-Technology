ACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= BaseApplication.h BallApp.h Physics.h

bin_PROGRAMS= BallApp
BallApp_CPPFLAGS= -I$(top_srcdir) -I/usr/include/bullet -I/usr/local/include/bullet
BallApp_SOURCES= BaseApplication.cpp BallApp.cpp Physics.cpp
BallApp_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS)
BallApp_LDADD= $(OGRE_LIBS) $(OIS_LIBS) -lboost_system -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
