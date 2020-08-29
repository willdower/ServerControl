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

#define PORT 80
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

    int masterSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (masterSocket < 0) {
        printf("socket() failed, exiting");
        exit(1);
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(masterSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0) {
        printf("Socket bind failed");
        exit(1);
    }
    else {
        printf("Socket bind successful\n");
    }

    if(listen(masterSocket, BACKLOG) != 0) {
        printf("Listening failed.");
        exit(1);
    }
    else {
        printf("Server listening...\n");
    }

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
            if ((newSocket = accept(masterSocket, (struct sockaddr *)&serverAddress, &addrlen)) < 0) {
                printf("Connection accept failure");
                exit(1);
            }
            else {
                printf("New client connected, socket is %d, ip is %s, port is %d\n", newSocket, inet_ntoa(serverAddress.sin_addr), ntohs(serverAddress.sin_port));
            }

            char *welcome = "Hello!\n";
            if (send(newSocket, welcome, strlen(welcome), 0) != strlen(welcome)) {
                printf("Failed to send welcome message.");
                exit(1);
            }
            else {
                printf("Welcome message sent successfully!\n");
            }

            for (int i=0;i<maxClients;i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = newSocket;
                    printf("Added to list of sockets at %d.\n", i);
                    break;
                }
            }
        }

        for (int i=0;i<maxClients;i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readset)) {
                if ((valread = read(sd, buffer, 1024)) == 0) {
                    // Disconnection
                    getpeername(sd, (struct sockaddr*)&serverAddress, &addrlen);
                    printf("Host disconnected, ip %s, port %d\n", inet_ntoa(serverAddress.sin_addr), ntohs(serverAddress.sin_port));
                    close(sd);
                    client_socket[i] = 0;
                }

                else {
                    buffer[valread] = '\0';
                    if (strcmp(buffer, "test\n") == 0) {
                        strcpy(buffer, "success\n");
                    }
                    else if (strcmp(buffer, "broadcast\n") == 0) {

                        for (int j=0;j<maxClients;j++) {
                            if (client_socket[j] == 0) {
                                continue;
                            }
                            strcpy(buffer, "A client has requested a broadcast.\n\0");
                            printf("Sending '%s' to client %d.\n", buffer, client_socket[j]);
                            send(client_socket[j], buffer, strlen(buffer), 0);
                        }
                        strcpy(buffer, "broadcast successful\n");
                    }
                    else {
                        continue;
                    }
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }

    }


    return 0;
}
