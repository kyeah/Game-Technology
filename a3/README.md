CS354R - Assignment 3 (Multiplayer Extension)
======================================================
Alyssa Sallean, Kevin Yeh, Patrick Grayson

BUILDING
=================================
1. Edit plugins.cfg so that the correct PluginsFolder variable is used.
2. Edit Makefile.am and make sure BallApp_CPPFLAGS is pointing to your bullet installation folder.
3. If building on OGRE 1.7, ensure that ogre.cfg does not exist or that the Fixed_Pipeline setting is removed before running the BallApp executable.
4. If you are using a recent Bullet Physics version, you may have to uncomment the provided code in src/Collisions.h.

DEPENDENCIES
=================================
sudo apt-get install libbullet-dev libsdl1.2-dev