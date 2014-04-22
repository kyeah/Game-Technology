#include <stdio.h>
#include "Sounds.h"
#include <boost/thread.hpp>

int Sounds::enabled = true;
Mix_Music* Sounds::mBackgroundMusic = NULL;


void Sounds::init(){
  //SETUP MUSIC

  int audio_rate = 44100;
  Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
  int audio_channels = 2;
  int audio_buffers = 4096;
  SDL_Init(SDL_INIT_AUDIO);

  if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
    printf("Unable to open audio!\n");
    exit(1);
  }
  
  Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);

}

void Sounds::playBackground(const char* aFilePath, int aVolume){
  //clamp volume
  if(aVolume > MAX_VOLUME) aVolume = 128;
  if(aVolume < MIN_VOLUME) aVolume = 0;

  mBackgroundMusic = NULL;
  mBackgroundMusic = Mix_LoadMUS(aFilePath);
  if(mBackgroundMusic != NULL){
    Mix_PlayMusic(mBackgroundMusic, -1);
    Mix_VolumeMusic(aVolume); 
    Mix_HookMusicFinished(musicDone); 
  }
}

void Sounds::musicDone() {
  Mix_HaltMusic();
  Mix_FreeMusic(mBackgroundMusic);
  mBackgroundMusic = NULL;
}

void Sounds::playSoundEffect(const char* aFilePath, int aVolume){
  //clamp volume
  if(aVolume > MAX_VOLUME) aVolume = 128;
  if(aVolume < MIN_VOLUME) aVolume = 0;

  Mix_Chunk* soundEffect = NULL;
  soundEffect = Mix_LoadWAV(aFilePath);
  if(soundEffect){
    int channel = Mix_PlayChannel(-1, soundEffect, 0);
    Mix_Volume(channel, aVolume);
    Mix_ChannelFinished(channelDone);
  }

}

void Sounds::channelDone(int aChannel){
  /* NOTE: if later we get sound crashes check this becasue all channles might be taken up and we're not freeing channels*/
}
