//
// Created by William on 29/08/2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <unistd.h>

#define BUF_SIZE 1025

extern int errno;

// Universal
int createSocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Failed to create socket, exiting.\n");
        exit(1);
    }
    return sock;
}

// Client Specific
void connectToServer(const int socket, struct sockaddr_in server, char *buf) {
    if (connect(socket, (struct sockaddr*)&server, sizeof(server)) == -1) {
        printf("Failed to connect to the server, exiting.\n");
        perror("");
        exit(1);
    }
}

void disconnectFromServer(const int socket) {
    close(socket);
}

// Server Specific
struct sockaddr_in setServerAddress(const int port) {
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
    return serverAddress;
}

void bindServerSocket(int socket, struct sockaddr_in address) {
    if (bind(socket, (struct sockaddr*)&address, sizeof(address)) != 0) {
        printf("Failed to bind socket to port, exiting.\n");
        perror("Error: ");
        exit(1);
    }
    else {
        printf("Bind successful.\n");
    }
}

void startListening(const int socket, const int backlogSize) {
    if(listen(socket, backlogSize) != 0) {
        printf("Failed to start listening, exiting.\n");
        perror("Error: ");
        exit(1);
    }
    else {
        printf("Server listening...\n");
    }
}

void handleNewConnection(const int masterSocket, struct sockaddr_in serverAddress, int *addrlen, int *client_socket, const int maxClients) {
    int newSocket;
    if ((newSocket = accept(masterSocket, (struct sockaddr *)&serverAddress, addrlen)) < 0) {
        printf("Failed to accept new connection, exiting.\n");
        perror("Error: ");
        exit(1);
    }
    else {
        printf("New client connected, socket is %d, ip is %s, port is %d.\n", newSocket, inet_ntoa(serverAddress.sin_addr), ntohs(serverAddress.sin_port));
    }

    char welcome[BUF_SIZE];
    strcpy(welcome, "Connected to server. Welcome.\n\0");
    if (send(newSocket, welcome, sizeof(char)*BUF_SIZE, 0) != sizeof(char)*BUF_SIZE) {
        printf("Failed to send welcome message, exiting.\n");
        perror("Error: ");
        exit(1);
    }
    else {
        printf("Successfully sent welcome message.\n");
    }

    for (int i=0;i<maxClients;i++) {
        if (client_socket[i] == 0) {
            client_socket[i] = newSocket;
            printf("Added new client to list of sockets at %d.\n", i);
            break;
        }
    }
}

void handleDisconnection(const int socket, struct sockaddr_in serverAddress, int *addrlen, int *client_socket, int socketLocation) {
    getpeername(socket, (struct sockaddr*)&serverAddress, addrlen);
    printf("Host on socket %d, disconnected, ip %s, port %d\n", socket, inet_ntoa(serverAddress.sin_addr), ntohs(serverAddress.sin_port));
    close(socket);
    client_socket[socketLocation] = 0;
}