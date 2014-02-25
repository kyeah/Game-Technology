#include <stdio.h>
#include "Sounds.h"

void Sounds::init(){
        //SETUP MUSIC
        int audio_rate = 22050;
        Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
        int audio_channels = 2;
        int audio_buffers = 4096;
        SDL_Init(SDL_INIT_AUDIO);
        /* This is where we open up our audio device.  Mix_OpenAudio takes
        as its parameters the audio format we'd /like/ to have. */
        if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
                printf("Unable to open audio!\n");
                exit(1);
        }
         /* If we actually care about what we got, we can ask here.  In this
        program we don't, but I'm showing the function call here anyway
        in case we'd want to know later. */
        Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
}

void Sounds::playSound(int sound_type, int volume){
	if(volume < 0 || volume > 128)
		printf("VOLUME TOO LOUD");
	else
		Mix_Volume(-1, volume);
	
	Mix_Music *sound = NULL;
        switch(sound_type){
                case 0:
                        sound = Mix_LoadMUS("media/sounds/swoosh-sound.mp3");
                        break;
                case 1:
                        sound = Mix_LoadMUS("media/sounds/hit-sound.mp3");
                        break;
		case 2: 
			//doesn't play in the background. 
			sound = Mix_LoadMUS("media/sounds/jazz.mp3");
			break;
                default:
                        printf("No Sound File Found\n");
        }
        if(sound != NULL){
                Mix_PlayMusic(sound, 0);
        }
}

