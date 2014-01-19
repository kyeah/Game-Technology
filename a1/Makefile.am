ACLOCAL_AMFLAGS= -I m4
noinst_HEADERS= BaseApplication.h BallApp.h

bin_PROGRAMS= BallApp
BallApp_CPPFLAGS= -I$(top_srcdir)
BallApp_SOURCES= BaseApplication.cpp BallApp.cpp
BallApp_CXXFLAGS= $(OGRE_CFLAGS) $(OIS_CFLAGS)
BallApp_LDADD= $(OGRE_LIBS) $(OIS_LIBS) -lboost_system

EXTRA_DIST= buildit makeit
AUTOMAKE_OPTIONS= foreign
