#include <btBulletDynamicsCommon.h>
#include "SDL.h"
#include <SDL_net.h>
#include "Networking.h"
#include "common.h"

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
  if (server_socket)
    SDLNet_TCP_Close(server_socket);
  
  if (client_socket)
    SDLNet_TCP_Close(client_socket);
  
  for (int i = 1; i < 4; i++) {
    Player *mPlayer = players[i];
    if (mPlayer && mPlayer->csd) {
      SDLNet_TCP_Close(mPlayer->csd);
    }
  }

  //  SDLNet_Quit();
}

std::vector<PingResponseMessage*> Networking::hostCheck( const char* filename ) {
  SDLNet_Init();

  std::ifstream ifs(filename);

  std::vector<PingResponseMessage*> messages;

  if (ifs) {
    std::string host;
    while (!ifs.eof()) {
      // Get next hostname
      std::getline(ifs, host);

      if (SDLNet_ResolveHost(&client_ip, host.c_str(), PORT) != -1) {
        client_socket = SDLNet_TCP_Open(&client_ip);
        if (client_socket) {
          std::cout << "Pinging hostname: " << host << std::endl;

          // Ping server for information
          PingMessage ping;
          ping.isJoining = false;
          SDLNet_TCP_Send(client_socket, (char*)&ping, sizeof(ping));

          PingResponseMessage *response = new PingResponseMessage();
          SDLNet_TCP_Recv(client_socket, response, sizeof(*response));

          std::cout << "Received lobby name: " << response->lobbyName << std::endl;
          strcpy(response->hostName, host.c_str());
          messages.push_back(response);
        }
      }
    }
  }

  return messages;
}

bool Networking::clientConnect(ConnectAck *ack, const char* username, char* host){

  printf("trying to connect to player 2...\n");
  SDLNet_Init();
  if(SDLNet_ResolveHost(&client_ip, host, PORT) == -1){
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(0);
    return false;
  }

  client_socket = SDLNet_TCP_Open(&client_ip);

  int retries = 0;
  while(!client_socket){
    retries++;
    if (retries >= 50) {
      return false;
    }

    client_socket = SDLNet_TCP_Open(&client_ip);
    if(!client_socket) {
      printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
      printf("trying again...\n");
    }
  }

  client_socketset = SDLNet_AllocSocketSet(1);
  if (SDLNet_TCP_AddSocket(client_socketset, client_socket) == -1) {
    printf("SDLNet_TCP_AddSocket: %s\n", SDLNet_GetError()); // Probably need to make the socketset bigger
    return false;
  }

  // Ping server for information
  PingMessage ping;
  ping.isJoining = true;
  strcpy(ping.name, username);
  SDLNet_TCP_Send(client_socket, (char*)&ping, sizeof(ping));

  //connected = true
  SDLNet_TCP_Recv(client_socket, ack, sizeof(*ack));

  for (int i = 0; i < 4; i++) {
    client_ids[i] = ack->ids[i];
  }
  //  printf("myID: %d\n", myId);
  return true;
}
