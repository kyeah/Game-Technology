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
  vector<ConfigNode*> extrudedMeshes;

  vector<string> planeMeshNames;
  vector<string> extrudedMeshNames;

  for(map<string, ConfigNode*>::iterator it = scripts.begin(); it != scripts.end(); ++it) {
    cout << it->first << endl;
    vector<string> name = split(it->first, ' ');
    cout << name[0] << " " << name[1] << endl;
    if (name[0].compare("planemesh") == 0) {
      planeMeshes.push_back(it->second);
      planeMeshNames.push_back(name[1]);
    } else if (name[0].compare("extrudedMesh") == 0) {
      extrudedMeshes.push_back(it->second);
      extrudedMeshNames.push_back(name[1]);
    } else if (name[0].compare("level") == 0) {
      levels.push_back(it->second);
      levelNames.push_back(name[1]);
    }
  }

  cout << "loading planes" << endl;
  loadPlaneMeshes(planeMeshes, planeMeshNames);
  cout << "loaded planes" << endl;
  loadExtrudedMeshes(extrudedMeshes, extrudedMeshNames);
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
    loadObject(objs[j]);
  }
}

void LevelLoader::loadPlaneMeshes(vector<ConfigNode*>& meshes, vector<string>& meshNames) {
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

void LevelLoader::loadExtrudedMeshes(vector<ConfigNode*>& meshes, vector<string>& meshNames) {
  for (int i = 0; i < meshes.size(); i++) {
    ConfigNode *root;
    ConfigNode *info[3];
    string ids[] = { "path", "shape", "track" };
    root = meshes[i];

    for (int i = 0; i < 3; i++) {
      info[i] = root->findChild(ids[i]);
    }

    if (!info[0] || !info[1]) continue;

    float utiles, vtiles;
    utiles = vtiles = 1.0;

    cout << "test" << endl;

    Procedural::Path p;
    parsePath(info[0], p);
    cout << "finished path" << endl;
    Procedural::Shape *s = parseShape(info[1]);
    cout << "finished shape" << endl;
    Procedural::Track *t = parseTrack(info[2]);
    cout << "finished track" << endl;
    Procedural::Extruder().setExtrusionPath(&p).setShapeToExtrude(s).setShapeTextureTrack(t).setUTile(utiles).setVTile(vtiles).realizeMesh(meshNames[i]);
    cout << "finished mesh" << endl;
  }
}

void LevelLoader::parsePath(ConfigNode *path, Procedural::Path& p) {
  ConfigNode *typeNode = path->findChild("type");
  if (typeNode) {
    string type = typeNode->getValue();

    if (type.compare("catmullSpline") == 0) {
      return parseCatmullSpline(path, p);
    } else if (type.compare("cubicHermiteSpline") == 0) {
      return parseCubicHermiteSpline(path, p);
    } else if (type.compare("kbSpline") == 0) {
      return parseKbSpline(path, p);
    } else if (type.compare("roundedCornerSpline") == 0) {
      return parseRoundedCornerSpline(path, p);
    } else if (type.compare("bezierCurve") == 0) {
      return parseBezierCurve(path, p);
    }
  }
}

void LevelLoader::parseCatmullSpline(ConfigNode *path, Procedural::Path& p) {
  Procedural::CatmullRomSpline3 *spline = new Procedural::CatmullRomSpline3();
  int segments = 8;
  bool close = true;

  ConfigNode *segNode = path->findChild("segments");
  if (segNode) segments = segNode->getValueI();
  spline->setNumSeg(segments);

  ConfigNode *pointsNode = path->findChild("points");
  if (pointsNode) {
    vector<ConfigNode*> points = pointsNode->getChildren();
    for (int i = 0; i < points.size(); i++) {
      btVector3 point = points[i]->getValueV3();
      spline->addPoint(point[0], point[1], point[2]);
    }
  }

  if (close) spline->close();
  p = spline->realizePath();
}

void LevelLoader::parseCubicHermiteSpline(ConfigNode *path, Procedural::Path& p) {

}

void LevelLoader::parseKbSpline(ConfigNode *path, Procedural::Path& p) {

}

void LevelLoader::parseRoundedCornerSpline(ConfigNode *path, Procedural::Path& p) {

}

void LevelLoader::parseBezierCurve(ConfigNode *path, Procedural::Path& p) {

}

Procedural::Shape* LevelLoader::parseShape(ConfigNode *path) {
  if (!path) return NULL;
  Procedural::Shape *shape = new Procedural::Shape();
  
  ConfigNode *outsideNode = path->findChild("outside");
  if (outsideNode && outsideNode->getValue().compare("left") == 0)
    shape->setOutSide(Procedural::SIDE_LEFT);
  
  ConfigNode *pointsNode = path->findChild("points");
  if (pointsNode) {
    vector<ConfigNode*> points = pointsNode->getChildren();
    for (int i = 0; i < points.size(); i++) {
      shape->addPoint(points[i]->getValueF(0), points[i]->getValueF(1));
    }
  }
  
  return shape;
}

Procedural::Track* LevelLoader::parseTrack(ConfigNode *path) {
  if (!path) return NULL;

  ConfigNode *amNode = path->findChild("addressingMode");
  Procedural::Track::AddressingMode am = Procedural::Track::AM_ABSOLUTE_LINEIC;
  if (amNode) {
    string am = amNode->getValue();
    if (am.compare("relative") == 0) {
      am = Procedural::Track::AM_RELATIVE_LINEIC;
    } else if (am.compare("point") == 0) {
      am = Procedural::Track::AM_POINT;
    }
  }

  Procedural::Track *track = new Procedural::Track(am);
  ConfigNode *keyNode = path->findChild("keyframes");
  if (keyNode) {
    vector<ConfigNode*> keys = keyNode->getChildren();
    for (int i = 0; i < keys.size(); i++) {
      track->addKeyFrame(keys[i]->getValueF(0), keys[i]->getValueF(1));
    }
  }

  return track;
}

void LevelLoader::loadObject(ConfigNode *obj) {
  string meshName, materialName;
  btVector3 scale(1,1,1);
  btVector3 startPos;
  btQuaternion startRot;

  vector<float> interpTimes;
  vector<btVector3> interpPos;

  vector<float> interpRotTimes;
  vector<btQuaternion> interpRot;

  bool ambient, diffuse, specular;
  ambient = diffuse = specular = false;
  float ar, ag, ab,
    dr, dg, db, da,
    sr, sg, sb, sa;

  vector<ConfigNode*> attrs = obj->getChildren();
  for (int i = 0; i < attrs.size(); i++) {
    string name = attrs[i]->getName();
    if (name.compare("meshname") == 0) {
      meshName = attrs[i]->getValue();

    } else if (name.compare("material") == 0) {
      materialName = attrs[i]->getValue();

    } else if (name.compare("scale") == 0) {
      scale = attrs[i]->getValueV3();

    } else if (name.compare("ambient") == 0) {
      ambient = true;
      ar = attrs[i]->getValueF(0);
      ag = attrs[i]->getValueF(1);
      ab = attrs[i]->getValueF(2);

    } else if (name.compare("diffuse") == 0) {
      diffuse = true;
      dr = attrs[i]->getValueF(0);
      dg = attrs[i]->getValueF(1);
      db = attrs[i]->getValueF(2);
      da = attrs[i]->getValueF(3);

    } else if (name.compare("specular") == 0) {
      specular = true;
      sr = attrs[i]->getValueF(0);
      sg = attrs[i]->getValueF(1);
      sb = attrs[i]->getValueF(2);
      sa = attrs[i]->getValueF(3);

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
  ss << "object" << id;
  string name = ss.str();
  id++;

  string type = obj->getName();
  GameObject *go;
  if (type.compare("plane") == 0) {
    go = new Plane(mSceneMgr, name, meshName, name, 0, mPhysics, startPos, scale,
                   btVector3(0,0,0), btScalar(0), btScalar(0.9), btVector3(0,0,0), &startRot);
  } else if (type.compare("extrudedObject") == 0) {
    go = new ExtrudedObject(mSceneMgr, name, meshName, name, 0, mPhysics, startPos, scale,
                            btVector3(0,0,0), btScalar(0), btScalar(0.9), btVector3(0,0,0), &startRot);
  }
    
  if (materialName.length() > 0)
    go->getEntity()->setMaterialName(materialName);

  go->setKinematic(true);
  go->setInterpTimes(interpTimes);
  go->setInterpPos(interpPos);
  go->setInterpRotTimes(interpRotTimes);
  go->setInterpRot(interpRot);

  if (ambient)  go->setAmbient(ar, ag, ab);
  if (diffuse)  go->setDiffuse(dr, dg, db, da);
  if (specular) go->setSpecular(sr, sg, sb, sa);
}
