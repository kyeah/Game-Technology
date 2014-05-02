#include <stdio.h>
#include <boost/thread.hpp>
#include "Sounds.h"
#include "OgreBallApplication.h"
#include "HostPlayerActivity.h"
#include "Networking.h"
#include "common.h"

int Sounds::volume = 64;
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

  if (mBackgroundMusic) {
    Mix_FreeMusic(mBackgroundMusic);
  }

  mBackgroundMusic = NULL;
  mBackgroundMusic = Mix_LoadMUS(aFilePath);
  if(mBackgroundMusic != NULL){
    Mix_FadeInMusic(mBackgroundMusic, -1, 1500);
    Mix_VolumeMusic(aVolume);
    //    Mix_HookMusicFinished(musicDone);
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

  Activity *a = OgreBallApplication::getSingleton()->activity;
  HostPlayerActivity *h = dynamic_cast<HostPlayerActivity*>(a);

  if (h) {
    ServerPacket soundPacket;
    soundPacket.type = SERVER_PLAY_SOUND;
    strcpy(soundPacket.msg, aFilePath);

    for (int j = 1; j < MAX_PLAYERS; j++) {
      if (players[j]) {
        Networking::Send(players[j]->csd, (char*)&soundPacket, sizeof(soundPacket));
      }
    }
  }
}

void Sounds::channelDone(int aChannel){
  /* NOTE: if later we get sound crashes check this becasue all channles might be taken up and we're not freeing channels*/
}
