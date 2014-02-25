#include <stdio.h>
#include "SDL.h"
#include "SDL_mixer.h"

class Sounds{
public:
	static Mix_Music *sound;
	static const int RACQUET_SWOOSH = 0;
	static const int BALL_HIT = 1;

	static void musicDone();
	static void init();
	static void playSound(int sound_type);
};


