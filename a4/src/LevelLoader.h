#pragma once

#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include "LevelViewer.h"
#include "Physics.h"
#include "ProceduralStableHeaders.h"
#include "Procedural.h"
#include "ProceduralUtils.h"
#include "../libs/ConfigLoader.hpp"

class LevelLoader {
  static LevelLoader* instance;

 public:
  LevelLoader(Ogre::SceneManager *mgr, Ogre::Camera *cam, Physics *phys, Ogre::SceneNode *levelRoot);

  static LevelLoader* getSingleton() { return instance; }
  static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
  static std::vector<std::string> split(const std::string &s, char delim);

  void clearKnobs();
  void setScene(Ogre::SceneManager *mgr, Ogre::Camera *cam, Physics *phys, Ogre::SceneNode *levelRoot);

  void loadResources(const std::string& path);  
  void loadLevel(const char* levelName);
  void loadLevel(LevelViewer *viewer, const char* levelName);

  void loadStartParameters(sh::ConfigNode *root);
  void loadLights(sh::ConfigNode *root);
  
  void loadPlaneMeshes(std::vector<sh::ConfigNode*>& meshes, std::vector<std::string> &meshNames);
  void loadExtrudedMeshes(std::vector<sh::ConfigNode*>& meshes, std::vector<std::string>& meshNames);
  
  void parsePath(sh::ConfigNode *path, Procedural::Path& p);
  Procedural::Shape* parseShape(sh::ConfigNode *shape);
  Procedural::Track* parseTrack(sh::ConfigNode *track);

  void loadObject(sh::ConfigNode *plane, Ogre::SceneNode *parentNode = NULL);

  void rotateLevel(btVector3 *axis, btScalar degree);

  Ogre::SceneManager *mSceneMgr;
  Ogre::SceneNode *levelRoot;
  Ogre::Camera *mCamera;
  Physics *mPhysics;

  Ogre::String mWinMusic;

  std::vector<sh::ConfigNode*> levels;
  std::vector<std::string> levelNames;

  // Start Parameters for most recently loaded level
  btVector3 playerStartPositions[4];
  btQuaternion playerStartRotations[4];

  // Camera Interpolations when level is the menu background
  std::vector<btVector3> camPosKnobs;
  std::vector<float> camPosInterpTimes;
  float totalCamPosInterpTime;

  std::vector<btVector3> camLookAtKnobs;
  std::vector<float> camLookAtInterpTimes;
  float totalCamLookAtInterpTime;

  float currentInterpCamPosTime;
  float currentInterpCamLookAtTime;
};
