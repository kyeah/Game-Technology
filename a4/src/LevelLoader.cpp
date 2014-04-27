#include <OgreMeshManager.h>
#include "GameObjectDescription.h"
#include "LevelLoader.h"
#include "Sounds.h"

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
  mLevelLoaded = false;
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

      stringstream ss;
      ss << name[1];
      for (int i = 2; i < name.size(); i++) {
        ss << " " << name[i];
      }
      levelNames.push_back(ss.str());
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

  mLevelLoaded = true;
  std::cout << "loading level!" << std::endl;
  numCollectibles = 0;

  for (int i = 0; i < levelNames.size(); i++) {
    if (levelNames[i].compare(levelName) == 0) {
      mCurrLevelID = i;
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

  // Yposition Cutoff
  ConfigNode *cutoffNode = root->findChild("fallCutoff");
  if (cutoffNode) {
    fallCutoff = cutoffNode->getValueF();
  } else {
    fallCutoff = -40000;
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

  // Sounds
  ConfigNode* pSound = root->findChild("Sound");

  if(pSound){
    ConfigNode* pBackground = pSound->findChild("background");
    if(pBackground){
      Ogre::String backgroundMusic = pBackground->getValue();
 //     Sounds::playBackground(backgroundMusic.c_str(), Sounds::MAX_VOLUME);
    }
  ConfigNode* pLevelID = root->findChild("levelID");

  if(pLevelID){
      levelID = pLevelID->getValueI();
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
      mSceneMgr->setSkyDome(true, skydomeNode->getValue(), skydomeNode->getValueF(1), skydomeNode->getValueF(2), skydomeNode->getValueF(3));
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

// I'm so sorry for the extruded mesh load functions. Godspeed.
void LevelLoader::loadExtrudedMeshes(vector<ConfigNode*>& meshes, vector<string>& meshNames) {
  for (int i = 0; i < meshes.size(); i++) {
    cout << "loading Extruded mesh " << meshNames[i] << endl;
    ConfigNode *root = meshes[i];

    if (!root->findChild("path") || !root->findChild("shape")) continue;

    float utiles, vtiles;
    utiles = vtiles = 1.0;

    Procedural::Path p, lastPath;
    Procedural::Shape s, lastShape;
    Procedural::MultiShape multishape;
    Procedural::Track scaleTrack, rotationTrack;
    bool useMultishape, useScaleTrack, useRotationTrack;
    useMultishape = useScaleTrack = useRotationTrack = false;

    s.close();
    vector<ConfigNode*> children = root->getChildren();
    for (int i = 0; i < children.size(); i++) {
      if (children[i]->getName().compare("path") == 0) {
        Procedural::Path pappend;

        ConfigNode *typeNode = children[i]->findChild("type");
        if (typeNode && typeNode->getValue().compare("previous") == 0) {
          pappend = lastPath;
        } else if (typeNode && typeNode->getValue().compare("previous-combined") == 0) {
          pappend = p;
        } else {
          parsePath(children[i], pappend);
        }

        ConfigNode *scaleNode = children[i]->findChild("scale");
        ConfigNode *translateNode = children[i]->findChild("translate");
        if (scaleNode) pappend.scale(scaleNode->getValueF(), scaleNode->getValueF(1), scaleNode->getValueF(2));
        if (translateNode) pappend.translate(translateNode->getValueF(), translateNode->getValueF(1), translateNode->getValueF(2));

        p.appendPath(pappend);
        lastPath = pappend;

      } else if (children[i]->getName().compare("shape") == 0) {
        Procedural::Shape sappend;

        ConfigNode *typeNode = children[i]->findChild("type");
        if (typeNode && typeNode->getValue().compare("previous") == 0) {
          sappend = lastShape;
        } else if (typeNode && typeNode->getValue().compare("previous-combined") == 0) {
          sappend = s;
        } else {
          parseShape(children[i], sappend);
        }

        ConfigNode *scaleNode = children[i]->findChild("scale");
        ConfigNode *rotateNode = children[i]->findChild("rotate");
        ConfigNode *translateNode = children[i]->findChild("translate");
        ConfigNode *mirrorNode = children[i]->findChild("mirror");
        ConfigNode *mirroraxisNode = children[i]->findChild("mirror-axis");
        if (scaleNode) sappend.scale(scaleNode->getValueF(), scaleNode->getValueF(1));
        if (rotateNode) sappend.rotate(Ogre::Degree(scaleNode->getValueF()));
        if (translateNode) sappend.translate(translateNode->getValueF(), translateNode->getValueF(1));
        if (mirrorNode) sappend.mirror(mirrorNode->getValueF(), mirrorNode->getValueF(1));
        if (mirroraxisNode) sappend.mirrorAroundAxis(Ogre::Vector2(mirrorNode->getValueF(), mirrorNode->getValueF(1)));

        ConfigNode *combineType = children[i]->findChild("combine");
        if (combineType) {
          string type = combineType->getValue();
          if (type.compare("union") == 0) {
            sappend.close();
            multishape = s.booleanUnion(sappend);
            useMultishape = true;
          } else if (type.compare("intersection") == 0) {
            sappend.close();
            multishape = s.booleanIntersect(sappend);
            useMultishape = true;
          }  else if (type.compare("difference") == 0) {
            sappend.close();
            multishape = s.booleanDifference(sappend);
            useMultishape = true;
          } else if (type.compare("scaleTrack") == 0) {
            useScaleTrack = true;
            scaleTrack = sappend.convertToTrack(Procedural::Track::AM_RELATIVE_LINEIC);
          } else if (type.compare("rotationTrack") == 0) {
            useRotationTrack = true;
            rotationTrack = sappend.convertToTrack(Procedural::Track::AM_RELATIVE_LINEIC);
          } else {
            s.appendShape(sappend);
          }
        } else {
          s.appendShape(sappend);
        }

        lastShape = sappend;
      }
    }

    ConfigNode *trackNode = root->findChild("track");
    Procedural::Track *t = parseTrack(trackNode);

    Ogre::Vector3 scale(1,1,1);

    ConfigNode *sNode = root->findChild("scale");
    if (sNode) {
      scale = Ogre::Vector3(sNode->getValueF(0), sNode->getValueF(1), sNode->getValueF(2));
    }

    ConfigNode *tileNode = root->findChild("tiles");
    if (tileNode) {
      utiles = tileNode->getValueI();
      vtiles = tileNode->getValueI(1);
    }

    ConfigNode *thickenNode = root->findChild("thicken");
    if (thickenNode) {
      multishape = s.thicken(thickenNode->getValueF());
      if (multishape.getShapeCount() > 1)
        useMultishape = true;
      else
        s = multishape.getShape(0);
    }

    s.close();

    if (useMultishape) {
      Procedural::Extruder().setExtrusionPath(&p).setScale(scale).setMultiShapeToExtrude(&multishape).setShapeTextureTrack(t).setUTile(utiles).setVTile(vtiles).realizeMesh(meshNames[i]);
    } else {
      Procedural::Extruder().setExtrusionPath(&p).setScale(scale).setShapeToExtrude(&s).setShapeTextureTrack(t).setUTile(utiles).setVTile(vtiles).realizeMesh(meshNames[i]);
    }
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
      Procedural::BezierCurve3 *spline = new Procedural::BezierCurve3();
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
    } else if (type.compare("helix") == 0) {
      Procedural::HelixPath *spline = new Procedural::HelixPath();

      ConfigNode *segNode = path->findChild("segments");
      if (segNode) segments = segNode->getValueI();
      spline->setNumSegPath(segments);

      ConfigNode *heightNode = path->findChild("height");
      if (heightNode) spline->setHeight(heightNode->getValueF());

      ConfigNode *radiusNode = path->findChild("radius");
      if (radiusNode) spline->setRadius(radiusNode->getValueF());

      ConfigNode *roundNode = path->findChild("rounds");
      if (roundNode) spline->setNumRound(roundNode->getValueF());

      p = spline->realizePath();
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
      Procedural::BezierCurve2 *spline = new Procedural::BezierCurve2();
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

    } else if (type.compare("kbSpline") == 0) {
      Procedural::KochanekBartelsSpline2 *spline = new Procedural::KochanekBartelsSpline2();
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

    } else if (type.compare("rectangle") == 0) {
      Procedural::RectangleShape *shape = new Procedural::RectangleShape();
      ConfigNode *widthNode = path->findChild("width");
      if (widthNode) shape->setWidth(widthNode->getValueF());

      ConfigNode *heightNode = path->findChild("height");
      if (heightNode) shape->setHeight(heightNode->getValueF());

      s = shape->realizeShape();

    } else if (type.compare("circle") == 0) {
      Procedural::CircleShape *shape = new Procedural::CircleShape();
      ConfigNode *radiusNode = path->findChild("radius");
      if (radiusNode) shape->setRadius(radiusNode->getValueF());

      ConfigNode *segNode = path->findChild("segments");
      if (segNode) shape->setNumSeg(segNode->getValueI());

      s = shape->realizeShape();

    }  else if (type.compare("ellipse") == 0) {
      Procedural::EllipseShape *shape = new Procedural::EllipseShape();
      ConfigNode *radiusNode = path->findChild("radius");
      if (radiusNode) {
        shape->setRadiusX(radiusNode->getValueF());
        shape->setRadiusY(radiusNode->getValueF(1));
      }

      ConfigNode *segNode = path->findChild("segments");
      if (segNode) shape->setNumSeg(segNode->getValueI());

      s = shape->realizeShape();

    } else if (type.compare("triangle") == 0) {
      Procedural::TriangleShape *shape = new Procedural::TriangleShape();
      ConfigNode *lengthNode = path->findChild("length");
      if (lengthNode) {
        if (lengthNode->getNumChildren() > 2) {
          shape->setLengthA(lengthNode->getValueF());
          shape->setLengthB(lengthNode->getValueF(1));
          shape->setLengthC(lengthNode->getValueF(2));
        } else {
          shape->setLength(lengthNode->getValueF());
        }
      }

      s = shape->realizeShape();

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
  Ogre::String soundEffect;

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
    }
    else if (name.compare("soundEffect") == 0)
      {
        soundEffect = attrs[i]->getValue();
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
                         btVector3(0,0,0), mass, rest, btVector3(0, 0, 0), &startRot, soundEffect);
    numCollectibles++;
  } else if (type.compare("extrudedObject") == 0) {
    go = new MeshObject(mSceneMgr, name, meshName, name, parentNode, mPhysics, startPos, scale,
                        btVector3(0,0,0), mass, rest, btVector3(0,0,0), &startRot);
  } else if (type.compare("goal") == 0) {
    go = new GoalObject(mSceneMgr, name, name, parentNode, mPhysics, startPos, scale,
                        btVector3(0,0,0), mass, rest, btVector3(0,0,0), &startRot, soundEffect);
  } else if (type.compare("collidable") == 0){
    go = new Collidable(mSceneMgr, name, meshName, name, parentNode, mPhysics, startPos, scale,
                        btVector3(0,0,0), mass, rest, btVector3(0,0,0), &startRot, soundEffect);
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
