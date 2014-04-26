#include <stdio.h>
#include "Sounds.h"

int Sounds::enabled = true;

void Sounds::init(){
  //SETUP MUSIC
  int audio_rate = 44100;
  Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
  int audio_channels = 2;
  int audio_buffers = 4096;
  mMusic = NULL;
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
  //Mix_AllocateChannels(5);
}

void Sounds::playBackground(){

  if(music == NULL){
    music = Mix_LoadMUS("media/OgreBall/sounds/background.mp3");
    Mix_PlayMusic(music, 0);
    Mix_HookMusicFinished(musicDone);
  }
  else
  {
    musicDone()
  }
  
}

void Sounds::musicDone() {
  Mix_HaltMusic();
  Mix_FreeMusic(music);
  music = NULL;
}

void Sounds::playSound(int sound_type, int volume){
  if (!enabled) return;

  if(volume < 0 || volume > 128)
    printf("VOLUME TOO LOUD");
  else
    Mix_Volume(-1, volume);

  Mix_Music *sound = NULL;
  switch(sound_type){
  case 0:
    return;
  case 1:
    sound = Mix_LoadMUS("media/sounds/swoosh-sound.mp3");
    break;
  case 2:
    sound = Mix_LoadMUS("media/sounds/hit-sound.mp3");
    break;
  case 3:
    //doesn't play in the background.
    sound = Mix_LoadMUS("media/sounds/jazz.mp3");
    break;
  case 4:
    //doesn't play in the background.
    sound = Mix_LoadMUS("media/sounds/point.mp3");
    break;
  default:
    printf("No Sound File Found\n");
  }
  if(sound != NULL){
    Mix_PlayMusic(sound, 0);
  }
}
