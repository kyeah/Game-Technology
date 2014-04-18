#include <OgreMeshManager.h>
#include "GameObjectDescription.h"
#include "LevelLoader.h"

using namespace std;
using namespace sh;

LevelLoader* LevelLoader::instance;

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

LevelLoader::LevelLoader(Ogre::SceneManager *mgr, Ogre::Camera *cam, Physics *phys, Ogre::SceneNode *lvlRoot) : mSceneMgr(mgr), mPhysics(phys), mCamera(cam), levelRoot(lvlRoot) { 
  instance = this;
}

void LevelLoader::setScene(Ogre::SceneManager *mgr, Ogre::Camera *cam, Physics *phys, Ogre::SceneNode *lvlRoot) { 
  mSceneMgr = mgr;
  mPhysics = phys;
  mCamera = cam;
  levelRoot = lvlRoot;
}

void LevelLoader::loadResources(const string& path) {
  ConfigLoader *mScriptLoader = new ConfigLoader(".ogreball");
  ConfigLoader::loadAllFiles(mScriptLoader, "media/OgreBall/scripts");

  map<string, ConfigNode*> scripts = mScriptLoader->getAllConfigScripts();
  vector<ConfigNode*> planeMeshes;
  vector<ConfigNode*> extrudedMeshes;

  vector<string> planeMeshNames;
  vector<string> extrudedMeshNames;

  for(map<string, ConfigNode*>::iterator it = scripts.begin(); it != scripts.end(); ++it) {
    vector<string> name = split(it->first, ' ');
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

  loadPlaneMeshes(planeMeshes, planeMeshNames);
  loadExtrudedMeshes(extrudedMeshes, extrudedMeshNames);
  cout << "Loaded Mesh scripts" << endl;
}

void LevelLoader::clearKnobs(void) {
  camPosKnobs.clear();
  camPosInterpTimes.clear();
  camLookAtKnobs.clear();
  camLookAtInterpTimes.clear();
  totalCamPosInterpTime = 0;
  totalCamLookAtInterpTime = 0;
  currentInterpCamPosTime = 0;
  currentInterpCamLookAtTime = 0;
}

void LevelLoader::loadLevel(LevelViewer *viewer, const char* levelName) {
  Ogre::SceneManager *mgr = mSceneMgr;
  Physics *physics = mPhysics;
  Ogre::Camera *cam = mCamera;
  Ogre::SceneNode *lvlRoot = levelRoot;
  
  setScene(viewer->mSceneMgr, viewer->mCamera, viewer->mPhysics, viewer->levelRoot);
  loadLevel(levelName);
  setScene(mgr, cam, physics, lvlRoot);
}

void LevelLoader::loadLevel(const char* levelName) {
  for (int i = 0; i < levelNames.size(); i++) {
    if (levelNames[i].compare(levelName) == 0) {
      ConfigNode *level = levels[i];
      vector<ConfigNode*> objs = level->getChildren();

      for (int j = 0; j < objs.size(); j++) {
        if (objs[j]->getName().compare("start") == 0) {
          loadStartParameters(objs[j]);
        } else if (objs[j]->getName().compare("lights") == 0) {
          loadLights(objs[j]);
        } else {
          loadObject(objs[j]);
        }
      }

      break;
    }
  }
}

void LevelLoader::loadStartParameters(ConfigNode *root) {

  // Initial camera positions and lookAts (i.e. for levelViewer)
  ConfigNode *camNode = root->findChild("camera");
  if (camNode) {
    ConfigNode *cposNode = camNode->findChild("pos");
    ConfigNode *clookNode = camNode->findChild("lookAt");
    
    if (cposNode) {
      btVector3 camPos = cposNode->getValueV3();
      mCamera->setPosition(camPos[0], camPos[1], camPos[2]);
      cameraStartPos = (Ogre::Vector3)camPos;
    }
    if (clookNode) {
      btVector3 clook = clookNode->getValueV3();
      mCamera->lookAt(clook[0], clook[1], clook[2]);
    }
  }

  // Waypoints
  ConfigNode *wpNode = root->findChild("waypoints");
  if (wpNode) {
    vector<ConfigNode*> wps = wpNode->getChildren();
    for (int i = 0; i< wps.size(); i++) {
      // Add new WaypointPlane that sets a player's restart point and rotation
      // if they fall off of the map or otherwise need to reset their position
    }
  }

  // Player Starting positions and rotations
  string ids[] = { "player1", "player2", "player3", "player4" };

  for (int i = 0; i < 4; i++) {
    ConfigNode *pNode = root->findChild(ids[i]);
    if (pNode) {
      ConfigNode *posNode = pNode->findChild("pos");
      ConfigNode *rotNode = pNode->findChild("rot");

      if (posNode) playerStartPositions[i] = posNode->getValueV3();
      if (rotNode) playerStartRotations[i] = rotNode->getValueYPR();
    }
  }

  // Skyboxes and Skydomes
  ConfigNode *skyboxNode = root->findChild("skybox");
  if (skyboxNode) {
    if (skyboxNode->getNumChildren() > 1) {
      // Distance; default 5000
      mSceneMgr->setSkyBox(true, skyboxNode->getValue(), skyboxNode->getValueF(1));
    } else {
      mSceneMgr->setSkyBox(true, skyboxNode->getValue());
    }
  }

  ConfigNode *skydomeNode = root->findChild("skydome");
  if (skydomeNode) {
    if (skydomeNode->getNumChildren() > 3) {
      // Curvature, Tiling, Distance; default 10-8-4000
      mSceneMgr->setSkyDome(true, skydomeNode->getValue(), skydomeNode->getValue(1), skydomeNode->getValue(2), skydomeNode->getValue(3));
    } else {
      mSceneMgr->setSkyDome(true, skydomeNode->getValue());
    }
  }
}

void LevelLoader::loadLights(ConfigNode *root) {
  vector<ConfigNode*> lightNodes = root->getChildren();
  for (int i = 0; i < lightNodes.size(); i++) {
    if (lightNodes[i]->getName().compare("ambient") == 0) {
      mSceneMgr->setAmbientLight(Ogre::ColourValue(lightNodes[i]->getValueF(0),
                                                   lightNodes[i]->getValueF(1),
                                                   lightNodes[i]->getValueF(2)));

    } else if (lightNodes[i]->getName().compare("shadow") == 0) {

    } else {
      Ogre::Light* light = mSceneMgr->createLight(lightNodes[i]->getName());

      vector<ConfigNode*> attrs = lightNodes[i]->getChildren();
      for (int i = 0; i < attrs.size(); i++) {
        string name = attrs[i]->getName();
        if (name.compare("pos") == 0) {
          btVector3 pos = attrs[i]->getValueV3();
          light->setPosition(pos[0], pos[1], pos[2]);

        } else if (name.compare("type") == 0) {
          if (attrs[i]->getValue().compare("directional") == 0) {
            light->setType(Ogre::Light::LT_DIRECTIONAL);
          } else if (attrs[i]->getValue().compare("spotlight") == 0) {
            light->setType(Ogre::Light::LT_SPOTLIGHT);
          }

        } else if (name.compare("diffuse") == 0) {
          light->setDiffuseColour(Ogre::ColourValue(attrs[i]->getValueF(0),
                                                    attrs[i]->getValueF(1),
                                                    attrs[i]->getValueF(2)));
        } else if (name.compare("specular") == 0) {
          light->setSpecularColour(Ogre::ColourValue(attrs[i]->getValueF(0),
                                                     attrs[i]->getValueF(1),
                                                     attrs[i]->getValueF(2)));

        } else if (name.compare("direction") == 0) {
          btVector3 dir = attrs[i]->getValueV3();
          light->setDirection(Ogre::Vector3(dir[0], dir[1], dir[2]));

        } else if (name.compare("range") == 0) {
          light->setSpotlightRange(Ogre::Degree(attrs[i]->getValueF(0)), Ogre::Degree(attrs[i]->getValueF(1)));

        }
      }
    }
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
    ConfigNode *root = meshes[i];

    if (!root->findChild("path") || !root->findChild("shape")) continue;

    float utiles, vtiles;
    utiles = vtiles = 1.0;

    Procedural::Path p;
    Procedural::Shape s;

    vector<ConfigNode*> children = root->getChildren();
    for (int i = 0; i < children.size(); i++) {
      if (children[i]->getName().compare("") == 0) {
        Procedural::Path pappend;
        parsePath(children[i], pappend);
        p.appendPath(pappend);
      } else if (children[i]->getName().compare("shape") == 0) {
        Procedural::Path sappend;
        parseShape(children[i], sappend);
        s.appendShape(sappend);
      }
    }

    ConfigNode *trackNode = root->findChild("track");
    Procedural::Track *t = parseTrack(info[2]);

    Ogre::Vector3 scale(1,1,1);

    ConfigNode *sNode = root->findChild("scale");
    if (sNode) {
      scale = Ogre::Vector3(sNode->getValueF(0), sNode->getValueF(1), sNode->getValueF(2));
    }

    Procedural::Extruder().setExtrusionPath(&p).setShapeToExtrude(&s).setShapeTextureTrack(t).setUTile(utiles).setVTile(vtiles).setScale(scale).realizeMesh(meshNames[i]);
  }
}

void LevelLoader::parsePath(ConfigNode *path, Procedural::Path& p) {
  ConfigNode *typeNode = path->findChild("type");
  if (typeNode) {
    int segments = 8;
    string type = typeNode->getValue();

    // Templates with no base constructors are stupid
    // Only the first line in each case statement is different
    // Please practice good coding practices when making libraries
    if (type.compare("catmullSpline") == 0) {
      Procedural::CatmullRomSpline3 *spline = new Procedural::CatmullRomSpline3();
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

      ConfigNode *closeNode = path->findChild("close");
      if (closeNode && closeNode->getValue().compare("true") == 0)
        spline->close();
      
      p = spline->realizePath();

    } else if (type.compare("cubicHermiteSpline") == 0) {
      Procedural::CubicHermiteSpline3 *spline = new Procedural::CubicHermiteSpline3();
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

      ConfigNode *closeNode = path->findChild("close");
      if (closeNode && closeNode->getValue().compare("true") == 0)
        spline->close();

      p = spline->realizePath();

    } else if (type.compare("roundedCornerSpline") == 0) {
      Procedural::RoundedCornerSpline3 *spline = new Procedural::RoundedCornerSpline3();
      ConfigNode *segNode = path->findChild("segments");
      if (segNode) segments = segNode->getValueI();
      spline->setNumSeg(segments);

      ConfigNode *radNode = path->findChild("radius");
      if (radNode) spline->setRadius(radNode->getValueF());

      ConfigNode *pointsNode = path->findChild("points");
      if (pointsNode) {
        vector<ConfigNode*> points = pointsNode->getChildren();
        for (int i = 0; i < points.size(); i++) {
          btVector3 point = points[i]->getValueV3();
          spline->addPoint(point[0], point[1], point[2]);
        }
      }

      ConfigNode *closeNode = path->findChild("close");
      if (closeNode && closeNode->getValue().compare("true") == 0)
        spline->close();

      p = spline->realizePath();

    } else if (type.compare("bezierCurve") == 0) {
      // We got the wrong version of OgreProcedural :(
    }
  } else {
    ConfigNode *pointsNode = path->findChild("points");
    if (pointsNode) {
      vector<ConfigNode*> points = pointsNode->getChildren();
      for (int i = 0; i < points.size(); i++) {
        btVector3 point = points[i]->getValueV3();
        p.addPoint(point[0], point[1], point[2]);
      }
    }
  }
}

void LevelLoader::parseShape(ConfigNode *path, Procedural::Shape& s) {
  ConfigNode *typeNode = path->findChild("type");
  if (typeNode) {
    int segments = 8;
    string type = typeNode->getValue();

    // Templates with no base constructors are stupid
    // Only the first line in each case statement is different
    // Please practice good coding practices when making libraries
    if (type.compare("catmullSpline") == 0) {
      Procedural::CatmullRomSpline2 *spline = new Procedural::CatmullRomSpline2();
      ConfigNode *segNode = path->findChild("segments");
      if (segNode) segments = segNode->getValueI();
      spline->setNumSeg(segments);

      ConfigNode *pointsNode = path->findChild("points");
      if (pointsNode) {
        vector<ConfigNode*> points = pointsNode->getChildren();
        for (int i = 0; i < points.size(); i++) {
          spline->addPoint(points[i]->getValueF(0), points[i]->getValueF(1));
        }
      }

      ConfigNode *closeNode = path->findChild("close");
      if (closeNode && closeNode->getValue().compare("true") == 0)
        spline->close();
      
      s = spline->realizeShape();

    } else if (type.compare("cubicHermiteSpline") == 0) {
      Procedural::CubicHermiteSpline2 *spline = new Procedural::CubicHermiteSpline2();
      ConfigNode *segNode = path->findChild("segments");
      if (segNode) segments = segNode->getValueI();
      spline->setNumSeg(segments);

      ConfigNode *pointsNode = path->findChild("points");
      if (pointsNode) {
        vector<ConfigNode*> points = pointsNode->getChildren();
        for (int i = 0; i < points.size(); i++) {
          spline->addPoint(points[i]->getValueF(0), points[i]->getValueF(1));
        }
      }

      ConfigNode *closeNode = path->findChild("close");
      if (closeNode && closeNode->getValue().compare("true") == 0)
        spline->close();

      s = spline->realizeShape();

    } else if (type.compare("roundedCornerSpline") == 0) {
      Procedural::RoundedCornerSpline2 *spline = new Procedural::RoundedCornerSpline2();
      ConfigNode *segNode = path->findChild("segments");
      if (segNode) segments = segNode->getValueI();
      spline->setNumSeg(segments);

      ConfigNode *radNode = path->findChild("radius");
      if (radNode) spline->setRadius(radNode->getValueF());

      ConfigNode *pointsNode = path->findChild("points");
      if (pointsNode) {
        vector<ConfigNode*> points = pointsNode->getChildren();
        for (int i = 0; i < points.size(); i++) {
          spline->addPoint(points[i]->getValueF(0), points[i]->getValueF(1));
        }
      }

      ConfigNode *closeNode = path->findChild("close");
      if (closeNode && closeNode->getValue().compare("true") == 0)
        spline->close();

      s = spline->realizeShape();

    } else if (type.compare("bezierCurve") == 0) {
      // We got the wrong version of OgreProcedural :(
    }
  } else {
    ConfigNode *pointsNode = path->findChild("points");
    if (pointsNode) {
      vector<ConfigNode*> points = pointsNode->getChildren();
      for (int i = 0; i < points.size(); i++) {
        s.addPoint(points[i]->getValueF(0), points[i]->getValueF(1));
      }
    }
  }

  ConfigNode *outsideNode = path->findChild("outside");
  if (outsideNode && outsideNode->getValue().compare("left") == 0)
    s.setOutSide(Procedural::SIDE_LEFT);
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

void LevelLoader::loadObject(ConfigNode *obj, Ogre::SceneNode *parentNode) {
  string meshName, materialName;
  btVector3 scale(1,1,1);
  btVector3 startPos;
  btQuaternion startRot;

  vector<float> interpTimes;
  vector<btVector3> interpPos;

  vector<float> interpRotTimes;
  vector<btQuaternion> interpRot;

  bool kinematic = true;
  float mass = 0.0f;
  float rest = 0.9f;

  bool ambient, diffuse, specular;
  ambient = diffuse = specular = false;
  float ar, ag, ab,
    dr, dg, db, da,
    sr, sg, sb, sa;

  vector<ConfigNode*> childObjects;

  vector<ConfigNode*> attrs = obj->getChildren();
  for (int i = 0; i < attrs.size(); i++) {
    string name = attrs[i]->getName();
    if (name.compare("meshname") == 0) {
      meshName = attrs[i]->getValue();

    } else if (name.compare("material") == 0) {
      materialName = attrs[i]->getValue();

    } else if (name.compare("scale") == 0) {
      scale = attrs[i]->getValueV3();

    } else if (name.compare("mass") == 0) {
      mass = attrs[i]->getValueF();

    } else if (name.compare("rest") == 0) {
      rest = attrs[i]->getValueF();

    } else if (name.compare("kinematic") == 0) {
      if (attrs[i]->getValue().compare("false") == 0) {
        kinematic = false;
      }

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
    } else {
      childObjects.push_back(attrs[i]);
    }
  }

  static int id = 0;
  stringstream ss;
  ss << "object" << id;
  string name = ss.str();
  id++;

  if (!parentNode && kinematic) {
    parentNode = levelRoot;
  }

  string type = obj->getName();
  GameObject *go;
  if (type.compare("plane") == 0) {
    go = new Plane(mSceneMgr, name, meshName, name, parentNode, mPhysics, startPos, scale,
                   btVector3(0,0,0), mass, rest, btVector3(0,0,0), &startRot);
  } else if (type.compare("collectible") == 0){
    go = new Collectible(mSceneMgr, name, meshName, name, parentNode, mPhysics, startPos, scale, 
                   btVector3(0,0,0), mass, rest, btVector3(0, 0, 0), &startRot);
  } else if (type.compare("extrudedObject") == 0) {
    go = new MeshObject(mSceneMgr, name, meshName, name, parentNode, mPhysics, startPos, scale,
                            btVector3(0,0,0), mass, rest, btVector3(0,0,0), &startRot);
  } else if (type.compare("goal") == 0) {
    go = new GoalObject(mSceneMgr, name, name, parentNode, mPhysics, startPos, scale,
                        btVector3(0,0,0), mass, rest, btVector3(0,0,0), &startRot);
  } else {
    go = new DecorativeObject(mSceneMgr, name, meshName, name, parentNode, mPhysics, startPos, scale, 
                              btVector3(0,0,0), mass, rest, btVector3(0, 0, 0), &startRot);
  }

  if (materialName.length() > 0)
    go->getEntity()->setMaterialName(materialName);

  if (kinematic) go->setKinematic(true);
  go->setInterpTimes(interpTimes);
  go->setInterpPos(interpPos);
  go->setInterpRotTimes(interpRotTimes);
  go->setInterpRot(interpRot);

  if (ambient)  go->setAmbient(ar, ag, ab);
  if (diffuse)  go->setDiffuse(dr, dg, db, da);
  if (specular) go->setSpecular(sr, sg, sb, sa);

  for (int i = 0; i < childObjects.size(); i++) {
    loadObject(childObjects[i], go->getNode());
  }
}

void LevelLoader::rotateLevel(btVector3 *axis, btScalar degree){
  btQuaternion q = btQuaternion(*axis, degree);
  levelRoot->rotate(Ogre::Quaternion(q.w(), q.x(), q.y(), q.z()));
}
