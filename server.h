//
// Created by William on 29/08/2020.
//

#ifndef SDCASSIGNMENT1_SERVER_H
#define SDCASSIGNMENT1_SERVER_H

int createSocket();
struct sockaddr_in setServerAddress(const int port);
void bindServerSocket(int socket, struct sockaddr_in address);
void startListening(const int socket, const int backlogSize);

void handleNewConnection(const int masterSocket, struct sockaddr_in serverAddress, int *addrlen, int *client_socket, const int maxClients);
void handleDisconnection(const int socket, struct sockaddr_in serverAddress, int *addrlen, int *client_socket, int socketLocation);

void getOS(char *string);
int getCores();
void *allocateSharedMemory(size_t size);

void sys(char *buffer, const int socket);
void put(const int socket, const int force, char *progname, const int files, int *sharedMem, int socketLoc);
void get(const int socket, char *progname, char *filename, int *sharedMem, const int socketLoc);

void receiveFile(char *progname, const int socket);
void sendFile(FILE *file, const int socket, char *filename);

#endif //SDCASSIGNMENT1_SERVER_H
