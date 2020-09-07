//
// Created by William on 29/08/2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>

#include "server.h"

#define PORT 80
#define BACKLOG 5
#define BUF_SIZE 1024
#define MAX_CLIENTS 30

int main() {
    sigaction(SIGCHLD, (const struct sigaction *) SIG_IGN, NULL);

    fd_set readset;
    char buffer[BUF_SIZE];
    int client_socket[MAX_CLIENTS];
    int maxClients = MAX_CLIENTS, maxSd, valread;

    int *sharedMemory = allocateSharedMemory(sizeof(client_socket));

    for (int i=0;i<maxClients;i++) {
        client_socket[i] = 0;
        sharedMemory[i] = 0;
    }


    int masterSocket = createSocket();

    struct sockaddr_in serverAddress = setServerAddress(PORT);

    bindServerSocket(masterSocket, serverAddress);

    startListening(masterSocket, BACKLOG);

    int addrlen = sizeof(serverAddress);

    while (1) {
        FD_ZERO(&readset);
        int sd, newSocket;

        FD_SET(masterSocket, &readset);
        maxSd = masterSocket;

        for (int i=0;i<maxClients;i++) {
            sd = client_socket[i];
            if (sd > 0) {
                FD_SET(sd, &readset);
            }
            if (sd > maxSd) {
                maxSd = sd;
            }
        }

        int activity = select(maxSd+1, &readset, NULL, NULL, NULL);

        if (FD_ISSET(masterSocket, &readset)) {
            handleNewConnection(masterSocket, serverAddress, &addrlen, client_socket, maxClients);
        }

        for (int i=0;i<maxClients;i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readset)) {
                if (sharedMemory[i] == 1) {
                    continue;
                }
                if ((valread = read(sd, buffer, BUF_SIZE)) <= 0) {
                    // Disconnection
                    handleDisconnection(sd, serverAddress, &addrlen, client_socket, i);
                }

                else {
                    // Handle input
                    if(buffer[0] == 'p' && buffer[1] == 'u' && buffer[2] == 't') {
                        put(sd, buffer, sharedMemory, i);
                    }
                    else if (buffer[0] == 'g' && buffer[1] == 'e' && buffer[2] == 't') {
                        get(sd, buffer, sharedMemory, i);
                    }
                    else if (buffer[0] == 'r' && buffer[1] == 'u' && buffer[2] == 'n') {
                        run(sd, buffer, sharedMemory, i);
                    }
                    else if (buffer[0] == 'l' && buffer[1] == 'i' && buffer[2] == 's' && buffer[3] == 't') {
                        list(buffer, sd);
                    }
                    else if (strcmp(buffer, "shutdown") == 0) {
                        disconnectAllClients(client_socket, maxClients);
                        sleep(1);
                        exit(0);
                    }
                    else if (strcmp(buffer, "sys") == 0) {
                        sys(buffer, sd);
                    }
                    else {
                        strcpy(buffer, "Unknown command, please try again.\n");
                        send(sd, buffer, sizeof(char)*BUF_SIZE, 0);
                    }
                }
            }
        }

    }
    return 0;
}
