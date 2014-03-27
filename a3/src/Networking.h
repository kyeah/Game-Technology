#ifndef NETWORKING_H
#define NETWORKING_H

#include <btBulletDynamicsCommon.h>
#include "RacquetObject.h"
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

//int MAX_PLAYERS = 4;
//static const int MAX_PLAYERS = 4;
/*const IPaddress serv_ip, client_ip;
const SDLNet_SocketSet client_socketset, server_socketset;
const int client_ids[4];
*/

typedef struct {
  int type;
  int userID;
  char msg[512];
  
  OIS::KeyCode keyArg;
  OIS::MouseState mouseArg;
  OIS::MouseButtonID mouseID;
} ClientPacket;

typedef struct {
  btVector3 nodePos;
  btQuaternion nodeOrientation;
} PlayerInfo;

typedef struct {
  int type;
  int clientId;
  char msg[512];
  int playSound;
  btVector3 ballPos;
  PlayerInfo players[4];
} ServerPacket;

typedef struct {
  int ids[4];
  int id;
} ConnectAck;

class Networking{
public:
	static void initSDLNet();
	static void Send(TCPsocket socket, char* msg, int len);
	static void serverConnect();
	static void Close();
	static bool clientConnect(); 

	static TCPsocket server_socket, client_socket;
	static IPaddress serv_ip, client_ip;
	static SDLNet_SocketSet client_socketset, server_socketset;
	static int client_ids[4];

};


#endif
