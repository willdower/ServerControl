//
// Created by William on 30/08/2020.
//

#ifndef SDCASSIGNMENT1_CLIENT_H
#define SDCASSIGNMENT1_CLIENT_H
int createSocket();
void connectToServer(const int socket, struct sockaddr_in server, char *buf);
void disconnectFromServer(const int socket);

void sendFile(FILE *file, const int socket, const int force, char *progname, char *filename);
#endif //SDCASSIGNMENT1_CLIENT_H
