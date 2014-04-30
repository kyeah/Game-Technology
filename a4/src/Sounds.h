#include <stdio.h>
#include <string>
#include "SDL.h"
#include "SDL_mixer.h"

class Sounds{
public:
	static const int NO_SOUND = 0;
	static const int RACQUET_SWOOSH = 1;
	static const int BALL_HIT = 2;
	static const int SMOOTH_JAZZ = 3;
	static const int SCORE_POINT = 4;
	static const int MAX_VOLUME = 128;
	static const int MIN_VOLUME = 0;
        static int volume;
        static int enabled;
        static Mix_Music* mBackgroundMusic;

	static void init();
	static void playBackground(const char*, int);
	static void playSoundEffect(const char*, int);
	static void musicDone();
	static void channelDone(int);
};

