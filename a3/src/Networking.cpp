#include <btBulletDynamicsCommon.h>
#include "SDL.h"
#include <SDL_net.h>
#include "Networking.h"


extern TCPsocket Networking::server_socket;
extern TCPsocket Networking::client_socket;
extern IPaddress Networking::serv_ip;
extern IPaddress Networking::client_ip;
extern SDLNet_SocketSet Networking::client_socketset;
extern SDLNet_SocketSet Networking::server_socketset;
extern int Networking::client_ids[4];
extern int Networking::soundState;
int PORT = 65505;

void Networking::initSDLNet() {
  if ( SDLNet_Init() < 0 ) {
    fprintf(stderr, "Couldn't initialize net: %s\n", SDLNet_GetError());
    SDL_Quit();
    exit(1);
  }
}


void Networking::Send(TCPsocket socket, char *msg, int len) {
        if (SDLNet_TCP_Send(socket, (void*)msg, len) < len) {
                printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
        }
}

void Networking::serverConnect(){
  SDLNet_Init();
  if(SDLNet_ResolveHost(&serv_ip, NULL, PORT) == -1) {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(0);
  }
  server_socket = SDLNet_TCP_Open(&serv_ip);
  if(!server_socket){
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(0);
  }

  server_socketset = SDLNet_AllocSocketSet(3);
}

void Networking::Close(){
        SDLNet_TCP_Close(server_socket);
        SDLNet_TCP_Close(client_socket);
        SDLNet_Quit();
}

bool Networking::clientConnect(int *id){
 printf("in connect\nPlease enter your hostname (default: pastamancer.cs.utexas.edu):");
  std::string host;
  getline(std::cin, host);
  if (host.length() == 0)
    host = std::string("pastamancer.cs.utexas.edu");

  printf("trying to connect to player 2...\n");
  SDLNet_Init();
  if(SDLNet_ResolveHost(&client_ip, host.c_str(), PORT) == -1){
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(0);
    return false;
  }

  client_socket = SDLNet_TCP_Open(&client_ip);
  while(!client_socket){
    client_socket = SDLNet_TCP_Open(&client_ip);
    if(!client_socket){
      printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
      printf("trying again...\n");
    }
  }

  client_socketset = SDLNet_AllocSocketSet(1);
  if (SDLNet_TCP_AddSocket(client_socketset, client_socket) == -1) {
    printf("SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError()); // Probably need to make the socketset bigger
    return false;
  }

//connected = true
  ConnectAck ack;
  SDLNet_TCP_Recv(client_socket, &ack, sizeof(ack));
  *id = ack.id;
  for (int i = 0; i < 4; i++) {
    client_ids[i] = ack.ids[i];
  }
//  printf("myID: %d\n", myId);
  return true;
}

