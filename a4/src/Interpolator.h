#pragma once

#include <btBulletDynamicsCommon.h>
#include <vector>

class Interpolator {
 public: 
  static btVector3 interpV3(float& currTime, float elapsedTime, float totalTime, 
                            std::vector<btVector3>& knobs, std::vector<float>& times, bool once=false);
  
  static btQuaternion interpQuat(float& currTime, float elapsedTime, float totalTime,
                                 std::vector<btQuaternion>& knobs, std::vector<float>& times, bool once=false);
    
};
