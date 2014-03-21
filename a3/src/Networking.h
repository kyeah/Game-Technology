#pragma once
#include <OISEvents.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include "SDL.h"
#include <SDL_net.h>

static const int MOUSE_MOVED = 0;
static const int MOUSE_PRESSED = 1;
static const int MOUSE_RELEASED = 2;
static const int KEY_PRESSED = 3;
static const int KEY_RELEASED = 4;
static const int CLIENT_CLOSE = 5;
static const int CLIENT_CHAT = 6;

static const int SERVER_CLIENT_CLOSED = 7;
static const int SERVER_CLOSED = 8;
static const int SERVER_UPDATE = 9;

typedef struct {
  int type;
  int userID;
  char msg[512];
  
  OIS::KeyCode keyArg;
  OIS::MouseState mouseArg;
  OIS::MouseButtonID mouseID;
} ClientPacket;

typedef struct {
  int type;
  int clientClosedId;
  char msg[512];
  btVector3 ballPos;
  btVector3 playerPos;  // TODO: Extend for multiple players; add doubles matches.
  btQuaternion playerOrientation;
} ServerPacket;

static void initSDLNet() {
  /* Initialize SDL */
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
    exit(1);
  }
  
  /* Initialize the network */
  if ( SDLNet_Init() < 0 ) {
    fprintf(stderr, "Couldn't initialize net: %s\n", SDLNet_GetError());
    SDL_Quit();
    exit(1);
  }
}
