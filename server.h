//
// Created by William on 29/08/2020.
//

#ifndef SDCASSIGNMENT1_SERVER_H
#define SDCASSIGNMENT1_SERVER_H

// networking.c
int createSocket();
struct sockaddr_in setServerAddress(const int port);
void bindServerSocket(int socket, struct sockaddr_in address);
void startListening(const int socket, const int backlogSize);
void handleNewConnection(const int masterSocket, struct sockaddr_in serverAddress, int *addrlen, int *client_socket, const int maxClients);
void handleDisconnection(const int socket, struct sockaddr_in serverAddress, int *addrlen, int *client_socket, int socketLocation);

// sys.c
void *allocateSharedMemory(size_t size);

// requests.c
void sys(char *buffer, const int socket);
void put(const int socket, char *command, int *sharedMem, int socketLoc);
void get(const int socket, char *command, int *sharedMem, const int socketLoc);
void run(const int socket, char *recv, int *sharedMem, const int socketLoc);
void list(char *command, const int socket);
void disconnectAllClients(int *clients, const int maxClients);

#endif //SDCASSIGNMENT1_SERVER_H
