#pragma once

#include <OgreSceneManager.h>
#include "Physics.h"
#include "../libs/ConfigLoader.hpp"

class LevelLoader {
 public:
  LevelLoader(Ogre::SceneManager *mgr, Physics *phys);

  static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
  static std::vector<std::string> split(const std::string &s, char delim);

  void loadResources(const std::string& path);
  void loadLevel(char* levelName);
  void loadPlaneMeshes(std::vector<sh::ConfigNode*>& meshes, std::vector<std::string> &meshNames);
  void loadPlane(sh::ConfigNode *plane);

  Ogre::SceneManager *mSceneMgr;
  Physics *mPhysics;

  std::vector<sh::ConfigNode*> levels;
  std::vector<std::string> levelNames;
};
