#pragma once

#include <OgreSceneManager.h>
#include "Physics.h"
#include "ProceduralStableHeaders.h"
#include "Procedural.h"
#include "ProceduralUtils.h"
#include "../libs/ConfigLoader.hpp"

class LevelLoader {
 public:
  LevelLoader(Ogre::SceneManager *mgr, Physics *phys);

  static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
  static std::vector<std::string> split(const std::string &s, char delim);

  void loadResources(const std::string& path);
  void loadLevel(char* levelName);
  
  void loadPlaneMeshes(std::vector<sh::ConfigNode*>& meshes, std::vector<std::string> &meshNames);
  void loadExtrudedMeshes(std::vector<sh::ConfigNode*>& meshes, std::vector<std::string>& meshNames);
  
  void parsePath(sh::ConfigNode *path, Procedural::Path& p);
  void parseCatmullSpline(sh::ConfigNode *path, Procedural::Path& p);
  void parseCubicHermiteSpline(sh::ConfigNode *path, Procedural::Path& p);
  void parseKbSpline(sh::ConfigNode *path, Procedural::Path& p);
  void parseRoundedCornerSpline(sh::ConfigNode *path, Procedural::Path& p);
  void parseBezierCurve(sh::ConfigNode *path, Procedural::Path& p);
  
  Procedural::Shape* parseShape(sh::ConfigNode *shape);
  Procedural::Track* parseTrack(sh::ConfigNode *track);

  void loadObject(sh::ConfigNode *plane);

  Ogre::SceneManager *mSceneMgr;
  Physics *mPhysics;

  std::vector<sh::ConfigNode*> levels;
  std::vector<std::string> levelNames;
};
