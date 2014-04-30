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

static const int MOUSE_MOVED = 0;
static const int MOUSE_PRESSED = 1;
static const int MOUSE_RELEASED = 2;
static const int KEY_PRESSED = 3;
static const int KEY_RELEASED = 4;
static const int CLIENT_CLOSE = 5;
static const int CLIENT_CHAT = 6;
static const int CLIENT_CLEAR_DIR = 7;

static const int SERVER_CLIENT_CONNECT = 8;
static const int SERVER_CLIENT_CLOSED = 9;
static const int SERVER_CLIENT_MESSAGE = 10;
static const int SERVER_CLOSED = 11;
static const int SERVER_UPDATE = 12;

static const int SEND_TO_SERV = 10;
static const int SEND_TO_CLIENT = 11;

typedef struct {
//TODO: Create Client Packet
} ClientPacket;

typedef struct {
//TODO: this may not even be necessary
} PlayerInfo;

typedef struct {
	int type;
	char level[128];
	int clientId;
//TODO: create server packet
} ServerPacket;

typedef struct {
  int ids[4];
  int id;
} ConnectAck;

typedef struct {
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
	static bool clientConnect(int *id, char* hostName); 
        static std::vector<PingResponseMessage*> hostCheck( const char* filename );

	static TCPsocket server_socket, client_socket;
	static IPaddress serv_ip, client_ip;
	static SDLNet_SocketSet client_socketset, server_socketset;
	static int client_ids[4];
	static int soundState;

};


#endif
