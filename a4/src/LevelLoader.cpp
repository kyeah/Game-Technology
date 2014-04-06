#include <OgreMeshManager.h>
#include "GameObjectDescription.h"
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

LevelLoader::LevelLoader(Ogre::SceneManager *mgr, Physics *phys) : mSceneMgr(mgr), mPhysics(phys) { }

void LevelLoader::loadResources(const string& path) {
  ConfigLoader *mScriptLoader = new ConfigLoader(".ogreball");
  ConfigLoader::loadAllFiles(mScriptLoader, "media/OgreBall/scripts");

  map<string, ConfigNode*> scripts = mScriptLoader->getAllConfigScripts();
  vector<ConfigNode*> planeMeshes;
  vector<string> planeMeshNames;

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

void LevelLoader::loadLevel(char* levelName) {
  int i;
  for (i = 0; i < levelNames.size(); i++) {
    if (levelNames[i].compare(levelName) == 0)
      break;
  }

  ConfigNode *level = levels[i];
  vector<ConfigNode*> objs = level->getChildren();

  for (int j = 0; j < objs.size(); j++) {
    string name = objs[j]->getName();
    if (name.compare("plane") == 0) {
      loadPlane(objs[j]);
    }
  }
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

void LevelLoader::loadPlane(ConfigNode *plane) {
  string meshName, materialName;
  btVector3 startPos;
  btQuaternion startRot;

  vector<float> interpTimes;
  vector<btVector3> interpPos;

  vector<float> interpRotTimes;
  vector<btQuaternion> interpRot;

  vector<ConfigNode*> attrs = plane->getChildren();
  for (int i = 0; i < attrs.size(); i++) {
    string name = attrs[i]->getName();
    if (name.compare("meshname") == 0) {
      meshName = attrs[i]->getValue();

    } else if (name.compare("material") == 0) {
      materialName = attrs[i]->getValue();

    } else if (name.compare("pos") == 0) {
      vector<ConfigNode*> pos = attrs[i]->getChildren();
      startPos = pos[0]->getValueV3();

      if (pos.size() > 1) {
        interpTimes.push_back(0);
        interpPos.push_back(startPos);

        for (int i = 1; i < pos.size(); i++) {
          interpTimes.push_back(pos[i]->getValueF(0));
          interpPos.push_back(pos[i]->getValueV3(1));
        }
      }

    } else if (name.compare("rot") == 0) {
      vector<ConfigNode*> rot = attrs[i]->getChildren();
      startRot = rot[0]->getValueYPR();

      if (rot.size() > 1) {
        interpRotTimes.push_back(0);
        interpRot.push_back(startRot);

        for (int i = 1; i < rot.size(); i++) {
          interpRotTimes.push_back(rot[i]->getValueF(0));
          interpRot.push_back(rot[i]->getValueYPR(1));
        }
      }
    }
  }

  static int id = 0;
  stringstream ss;
  ss << "plane" << id;
  string name = ss.str();

  Plane *p = new Plane(mSceneMgr, name, meshName, name, 0, mPhysics, startPos,
                       btVector3(0,0,0), btScalar(1), btScalar(0.9), btVector3(0,0,0), &startRot);

  if (materialName.length() > 0)
    p->getEntity()->setMaterialName(materialName);

  p->setKinematic(true);
  p->setInterpTimes(interpTimes);
  p->setInterpPos(interpPos);
  p->setInterpRotTimes(interpRotTimes);
  p->setInterpRot(interpRot);
}
