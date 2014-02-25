#include <stdio.h>
#include "Sounds.h"

Mix_Music *Sounds::sound = NULL;

void Sounds::musicDone(){
        Mix_HaltMusic();
        Mix_FreeMusic(sound);
        sound = NULL;
} 

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
        sound = NULL;
         /* If we actually care about what we got, we can ask here.  In this
        program we don't, but I'm showing the function call here anyway
        in case we'd want to know later. */
        Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
}

void Sounds::playSound(int sound_type){
	printf("trying to play sound...\n");
        switch(sound_type){
                case 0:
			printf("trying to play swoosh...\n");
                        sound = Mix_LoadMUS("media/sounds/swoosh-sound.mp3");
                        break;
                case 1:
			printf("trying to play hit...\n");
                        sound = Mix_LoadMUS("media/sounds/hit-sound.mp3");
                        break;
                default:
                        printf("No Sound File Found\n");
        }
        if(sound != NULL){
		printf("sound isn't null...\n");
                Mix_PlayMusic(sound, 0);
                Mix_HookMusicFinished(musicDone);
        }
}

