#pragma once
#include <pthread.h>

typedef struct {
  CEGUI::AnimationInstance *instance;
  float delay;
} animDelayArgs;

class OBAnimationManager {
 public:
  static void *delayStart(void *params) {
    animDelayArgs *args = (animDelayArgs*)params;
    usleep(args->delay*1000000.0f);
    if (args->instance)
      args->instance->start();
  }

  static CEGUI::AnimationInstance* startAnimation(const char* animName, CEGUI::Window* window, float speed=1.0, float delay=0.0) {
    CEGUI::AnimationManager *mgr = CEGUI::AnimationManager::getSingletonPtr();
    CEGUI::AnimationInstance* instance = mgr->instantiateAnimation(mgr->getAnimation(animName));
    instance->setTarget(window);
    instance->setSpeed(speed);

    if (delay) {
      animDelayArgs *args = (animDelayArgs*)malloc(sizeof(animDelayArgs));
      args->instance = instance;
      args->delay = delay;
      pthread_t thread;
      pthread_create(&thread, 0, delayStart, (void*)args);
    } else {
      instance->start();
    }
    return instance;
  }
};
