//
// Created by William on 7/09/2020.
//

#ifndef SDCASSIGNMENT1_REQUESTS_H
#define SDCASSIGNMENT1_REQUESTS_H

// sys.c
void getOS(char *string);
int getCores();
void receiveFile(char *progname, const int socket);
void sendFile(FILE *file, const int socket, char *filename);
void makeProgram(char *progname);

#endif //SDCASSIGNMENT1_REQUESTS_H
