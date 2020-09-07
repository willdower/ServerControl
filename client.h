//
// Created by William on 30/08/2020.
//

#ifndef SDCASSIGNMENT1_CLIENT_H
#define SDCASSIGNMENT1_CLIENT_H

// networking.c
int createSocket();
void connectToServer(const int socket, struct sockaddr_in server, char *buf);
void disconnectFromServer(const int socket);

// commands.c
void getCommand(char *command, const int socket);
void putCommand(char *command, struct sockaddr_in server);
void runCommand(char *command, struct sockaddr_in server);
void listCommand(char *command, const int socket);

#endif //SDCASSIGNMENT1_CLIENT_H
