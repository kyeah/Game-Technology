#include "Interpolator.h"

btVector3 Interpolator::interpV3(float& currTime, float elapsedTime, float totalTime,
                                 std::vector<btVector3>& knobs, std::vector<float>& times, bool once) {

  if (knobs.size() > 0) {
    currTime += elapsedTime;

    if (once && currTime >= totalTime) {
      currTime = totalTime;
      return knobs[knobs.size() - 1];
    }

    currTime = fmod(currTime, totalTime);

    for (int i = 1; i < times.size(); i++) {
      if (currTime < times[i]) {
        // Interp time is between position (i-1) and position i
        btVector3 first = knobs[i-1];
        btVector3 second = knobs[i];

        float dt = currTime - times[i-1];
        float proportion = dt/(times[i]-times[i-1]);

        btVector3 v = first.lerp(second, proportion);
        return v;
      }
    }
  }

  return btVector3(0,0,0);
}

btQuaternion Interpolator::interpQuat(float& currTime, float elapsedTime, float totalTime,
                                      std::vector<btQuaternion>& knobs, std::vector<float>& times, bool once) {

  if (knobs.size() > 0) {
    currTime += elapsedTime;

    if (once && currTime >= totalTime) {
      currTime = totalTime;
      return knobs[knobs.size() - 1];
    }

    currTime = fmod(currTime, totalTime);

    for (int i = 1; i < times.size(); i++) {
      if (currTime < times[i]) {
        // Interp time is between position (i-1) and position i
        btQuaternion first = knobs[i-1];
        btQuaternion second = knobs[i];

        float dt = currTime - times[i-1];
        float proportion = dt/(times[i]-times[i-1]);

        btQuaternion q = first.slerp(second, proportion);
        return q;
      }
    }
  }

  return btQuaternion(0,0,0,0);
}
