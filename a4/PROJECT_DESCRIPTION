CS 354R - The Final Frontier (MAKE YOUR OWN GODDAMN GAME)
===========================================================

RUNNING:
============
1. Consult README.md for instructions on building.
2. Run the ./OgreBall Executable.

CONTROLS
===========
Keyboard:
WASD - Level Tilt
ESC - Menu

Gamecube USB:
Left Stick - Level Tilt

Single Player:
R - Restart (Use a life)

Multi Player:
C - Toggle Chatlog Visibility
Enter - Toggle Message
ESC - Menu / Exit Message Context

FINAL IMPLEMENTATION OVERVIEW
===============================
Swanky New Game Engine Features:
1. Ogre Procedural Integration (Procedurally Generated Meshes)
2. Level and Mesh-building Scripting Language and Parser (stored in media/OgreBall/scripts/)
3. Single Player Leaderboards (persisted in data/*.obhs)
4. 4-Player Network Support with User Tags and Chatlog
5. Swanky Menus and Gamestate Screens + CEGUI Animations
6. Interpolated Objects for Dynamic Levels
7. USB GAMECUBE CONTROLLER SUPPORT FOR SINGLE PLAYER WHAT UP
8. Smart camera
9. More cool stuff we probably forgot about

FILES
=======
common.*

BaseApplication.*, OgreBallApplication.* (Main Application)

Activity.*, MenuActivity.*, SinglePlayerActivity.*, BaseMultiActivity.*, HostPlayerActivity.*, ClientPlayerActivity.* (Sub-Application States)

CameraObject.* (Smart Camera Positioning)
LevelLoader.* (Level and Mesh-building Script Parser)
LevelViewer.* (Ogre-Camera Viewers in CEGUI Windows)

Interpolator.*, OBAnimationManager.* (Dynamic Helpers)
SelectorHelper.* (Menu Select Helpers)

GameObject.*, GameObjectDescriptions.h, Collectible.cpp, GoalObject.cpp, OgreBall.cpp, DecorativeObject.cpp, MeshObject.cpp (Game Object Definitions)

Physics.*, OgreMotionState.* Sounds.*, Networking.* (Library Integration Helpers)