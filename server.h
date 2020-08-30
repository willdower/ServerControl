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
#endif //SDCASSIGNMENT1_SERVER_H
