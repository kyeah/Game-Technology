#ifndef NETWORKING_H
#define NETWORKING_H

#include <btBulletDynamicsCommon.h>
//#include "RacquetObject.h"
#include <iostream>
#include <string>
#include "SDL.h"
#include <SDL_net.h>
#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISMouse.h>
#include <OISKeyboard.h>
#include <OgreSceneManager.h>

static const int MOUSE_MOVED = 0;
static const int MOUSE_PRESSED = 1;
static const int MOUSE_RELEASED = 2;
static const int KEY_PRESSED = 3;
static const int KEY_RELEASED = 4;
static const int CLIENT_CLOSE = 5;
static const int CLIENT_CHAT = 6;
static const int CLIENT_CLEAR_DIR = 7;
static const int CLIENT_TOGGLEREADY = 77;

static const int SERVER_CLIENT_CONNECT = 8;
static const int SERVER_CLIENT_CLOSED = 9;
static const int SERVER_CLIENT_MESSAGE = 10;
static const int SERVER_CLOSED = 11;
static const int SERVER_UPDATE = 12;
static const int SERVER_GAME_START = 13;
static const int SERVER_CROSSED_FINISH_LINE = 14;
static const int SERVER_PLAYER_MESH_CHANGE = 15;
static const int SERVER_LEVEL_CHANGE = 16;

static const int SEND_TO_SERV = 10;
static const int SEND_TO_CLIENT = 11;

typedef struct {
  int type;
  int userID;
  char msg[512];
  
  OIS::KeyCode keyArg;
  OIS::MouseState mouseArg;
  OIS::MouseButtonID mouseID;

} ClientPacket;

typedef struct {
  char name[128];
  int characterChoice;
} PlayerInfo;

typedef struct {
  Ogre::Vector3 position;
  Ogre::Quaternion orientation;
} PlayerCamInfo;

typedef struct {
  btVector3 position;
  btQuaternion orientation;
} ObjectInfo;

typedef struct {
  int type;
  int clientID;
  float timeLeft;
  char msg[512];

  int playSound;
  ObjectInfo objectInfo[200];  // Should correspond to objList from Physics instance
  PlayerCamInfo camInfo;
} ServerPacket;

typedef struct {
  int id;
  int ids[4];
  PlayerInfo playerInfo[4];

  char level[128];
  char lobbyName[128];
} ConnectAck;

typedef struct {
  char name[128];
  bool isJoining;
} PingMessage;

typedef struct {
  char hostName[128];
  char lobbyName[128];
  int numPlayers;
  int maxPlayers;
} PingResponseMessage;

class Networking{
public:
	static void initSDLNet();
	static void Send(TCPsocket socket, char* msg, int len);
	static void serverConnect();
	static void Close();
	static bool clientConnect(ConnectAck *ack, const char* username, char* hostName); 
        static std::vector<PingResponseMessage*> hostCheck( const char* filename );

	static TCPsocket server_socket, client_socket;
	static IPaddress serv_ip, client_ip;
	static SDLNet_SocketSet client_socketset, server_socketset;
	static int client_ids[4];
	static int soundState;
};


#endif
