//
// Created by William on 7/09/2020.
//

#ifndef SDCASSIGNMENT1_COMMANDS_H
#define SDCASSIGNMENT1_COMMANDS_H

// networking.c
int createSocket();
void connectToServer(const int socket, struct sockaddr_in server, char *buf);

// sys.c
void receiveOnClient(FILE *file, const int socket);
void sendFile(FILE *file, const int socket, char *filename);
void getFilenameFromPath(char *path);

#endif //SDCASSIGNMENT1_COMMANDS_H
