#include <stdio.h>
#include "SDL.h"
#include "SDL_mixer.h"

class Sounds{
public:
	static const int NO_SOUND = 0;
	static const int RACQUET_SWOOSH = 1;
	static const int BALL_HIT = 2;
	static const int SMOOTH_JAZZ = 3;
	static const int SCORE_POINT = 4;
        static int enabled;

	static void musicDone();
	static void init();
	static void playSound(int sound_type, int volume);
};


