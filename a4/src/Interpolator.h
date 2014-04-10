#pragma once

#include <btBulletDynamicsCommon.h>
#include <vector>

class Interpolator {
 public: 

  // Use Bullet interpolation with proportions bound below 1
  static btVector3 interpV3(float& currTime, float elapsedTime, float totalTime,
                            btVector3 first, btVector3 second);
  static btQuaternion interpQuat(float& currTime, float elapsedTime, float totalTime,
                                 btQuaternion first, btQuaternion second);

  // Interpolate between a series of knobs; use once to avoid wrap-around.
  static btVector3 interpV3(float& currTime, float elapsedTime, float totalTime, 
                            std::vector<btVector3>& knobs, std::vector<float>& times, bool once=false);
  
  static btQuaternion interpQuat(float& currTime, float elapsedTime, float totalTime,
                                 std::vector<btQuaternion>& knobs, std::vector<float>& times, bool once=false);
    
};
