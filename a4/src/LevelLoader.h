#pragma once

#include "../libs/ConfigLoader.hpp"

class LevelLoader {
 public:
  static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
  static std::vector<std::string> split(const std::string &s, char delim);
  
  static void loadLevels(const std::string& path);
  static void loadPlaneMeshes(std::vector<sh::ConfigNode*>& meshes, std::vector<std::string> &meshNames);
};
