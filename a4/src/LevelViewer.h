#pragma once

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
#include "Physics.h"

class LevelViewer {
 public:
  LevelViewer( CEGUI::OgreRenderer *mRenderer, const char* levelName );
  ~LevelViewer(void);
  bool frameStarted( Ogre::Real elapsedTime );

  void loadLevel(const char* levelName);
  void setPositionPercent(float x, float y);

  Ogre::SceneManager *mSceneMgr;
  Ogre::SceneNode *levelRoot;
  Ogre::Camera *mCamera;
  Physics *mPhysics;

  std::string myCeguiImageName;
  CEGUI::Imageset *myImageSet;
  CEGUI::Window *window;

  // Camera Interpolation Information
  std::vector<btVector3> camPosKnobs;
  std::vector<float> camPosInterpTimes;
  float totalCamPosInterpTime;

  std::vector<btVector3> camLookAtKnobs;
  std::vector<float> camLookAtInterpTimes;
  float totalCamLookAtInterpTime;

  float currentInterpCamPosTime;
  float currentInterpCamLookAtTime;
};
