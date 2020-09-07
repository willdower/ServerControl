//
// Created by William on 30/08/2020.
//

#ifndef SDCASSIGNMENT1_CLIENT_H
#define SDCASSIGNMENT1_CLIENT_H
int createSocket();
void connectToServer(const int socket, struct sockaddr_in server, char *buf);
void disconnectFromServer(const int socket);

void getFilenameFromPath(char *path);

void sendFile(FILE *file, const int socket, char *filename);
void receiveOnClient(FILE *file, const int socket);
#endif //SDCASSIGNMENT1_CLIENT_H
