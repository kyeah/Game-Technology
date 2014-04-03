#include <OgreMeshManager.h>
#include "LevelLoader.h"

using namespace std;
using namespace sh;

vector<string>& LevelLoader::split(const string &s, char delim, vector<string> &elems) {
  stringstream ss(s);
  string item;
  while (getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}


vector<string> LevelLoader::split(const string &s, char delim) {
  vector<string> elems;
  split(s, delim, elems);
  return elems;
}

void LevelLoader::loadLevels(const string& path) {
  ConfigLoader *mScriptLoader = new ConfigLoader(".ogreball");
  ConfigLoader::loadAllFiles(mScriptLoader, "media/OgreBall/scripts");

  map<string, ConfigNode*> scripts = mScriptLoader->getAllConfigScripts();
  vector<ConfigNode*> planeMeshes;
  vector<ConfigNode*> levels;

  vector<string> planeMeshNames;
  vector<string> levelNames;

  for(map<string, ConfigNode*>::iterator it = scripts.begin(); it != scripts.end(); ++it) {
    vector<string> name = split(it->first, ' ');
    if (name[0].compare("planemesh") == 0) {
      planeMeshes.push_back(it->second);
      planeMeshNames.push_back(name[1]);
    } else if (name[0].compare("level") == 0) {
      levels.push_back(it->second);
      levelNames.push_back(name[1]);
    }
  }
  
  loadPlaneMeshes(planeMeshes, planeMeshNames);
  
}

void LevelLoader::loadPlaneMeshes(vector<ConfigNode*>& meshes, vector<std::string>& meshNames) {
  for (int i = 0; i < meshes.size(); i++) {
    ConfigNode *root;
    ConfigNode *info[5];
    string ids[] = { "dir", "size", "up", "segments", "texRepeat" };
    root = meshes[i];

    for (int i = 0; i < 5; i++) {
      info[i] = root->findChild(ids[i]);
    }

    Ogre::Plane p = Ogre::Plane(Ogre::Vector3(info[0]->getValueF(0), info[0]->getValueF(1), info[0]->getValueF(2)),
                                info[0]->getValueF(3));

    Ogre::MeshManager::getSingleton().createPlane(meshNames[i], Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                  p, info[1]->getValueF(0), info[1]->getValueF(1),
                                                  info[3]->getValueF(0), info[3]->getValueF(1), true, 1,
                                                  info[4]->getValueF(0), info[4]->getValueF(1),
                                                  Ogre::Vector3(info[2]->getValueF(0),
                                                                info[2]->getValueF(1),
                                                                info[2]->getValueF(2)));
  }
}
