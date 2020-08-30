//
// Created by William on 29/08/2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"

#define PORT 9122
#define BACKLOG 5
#define BUF_SIZE 1025
#define MAX_CLIENTS 30

int main() {

    fd_set readset;
    char buffer[BUF_SIZE];
    int client_socket[MAX_CLIENTS];
    int maxClients = MAX_CLIENTS, maxSd, valread;

    for (int i=0;i<maxClients;i++) {
        client_socket[i] = 0;
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
                if ((valread = read(sd, buffer, 1024)) == 0) {
                    // Disconnection
                    handleDisconnection(sd, serverAddress, &addrlen, client_socket, i);
                }

                else {
                    // Handle input
                    if (strcmp(buffer, "shutdown\n") == 0) {
                        printf("Shutdown command received, shutting down.\n");
                        exit(0);
                    }
                    else if (strcmp(buffer, "sys\n") == 0) {
                        getOS(buffer);
                        send(sd, buffer, sizeof(buffer), 0);
                    }
                }
            }
        }

    }


    return 0;
}
