#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include "client.h"

#define BUF_SIZE 1025

void sigpipeHandler(int unusuedVar) {
    printf("Connection broken, exiting.\n");
    exit(0);
}

void putCommand(char *command, const int socket) {
    int force = 0;
    char original_command[BUF_SIZE], progname[BUF_SIZE];
    strcpy(original_command, command);
    char *token = strtok(command, " ");
    while (token != NULL) {
        if (strcmp(token, "-f") == 0) {
            force = 1;
            break;
        }
        token = strtok(NULL, " ");
    }

    strcpy(command, original_command);

    token = strtok(command, " ");
    token = strtok(NULL, " ");

    int progDone = 0;
    while (token != NULL) {
        if (strcmp(token, "-f") == 0) {
            token = strtok(NULL, " ");
            continue;
        }
        if (progDone == 0) {
            strcpy(progname, token);
            token = strtok(NULL, " ");
            progDone = 1;
            continue;
        }

        FILE *file = fopen(token, "r");
        sendFile(file, socket, force, progname, token);
        fclose(file);
        token = strtok(NULL, " ");
    }
}

int main(int argc, char **argv) {
    sigaction(SIGPIPE, &(struct sigaction){sigpipeHandler}, NULL);

    fd_set readset, consoleset;
    struct sockaddr_in server;
    int sd = createSocket();
    struct hostent *hp = gethostbyname(argv[1]); // Convert string of hostname to hostname struct

    char command[BUF_SIZE], receive_buf[BUF_SIZE];

    bcopy(hp->h_addr, &(server.sin_addr.s_addr), hp->h_length);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2])); // Convert string of port into int and pass to sin_port

    connectToServer(sd, server, receive_buf);

    for(;;) {
        FD_ZERO(&readset);
        FD_SET(sd, &readset);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100;
        select(sd+1, &readset, NULL, NULL, &tv);

        if (FD_ISSET(sd, &readset)) {
            // Handle input from server

            if (read(sd, receive_buf, sizeof(receive_buf)) == 0) {
                printf("Server has closed connection. Exiting.\n");
                exit(0);
            }
            if (strcmp(receive_buf, "stillalive") == 0) {
                // Ignore
            }
            else if (strcmp(receive_buf, "Shutdown command received, server is shutting down.\n") == 0) {
                printf("%s\n", receive_buf);
                printf("Client will now disconnect.");
                fflush(stdout);
                exit(0);
            }
            else {
                printf("%s\n", receive_buf);
                fflush(stdout);
            }
        }

        tv.tv_sec = 0;
        tv.tv_usec = 100;

        FD_ZERO(&consoleset);
        FD_SET(fileno(stdin), &consoleset);
        int num;
        if ((num = select(fileno(stdin)+1, &consoleset, NULL, NULL, &tv)) == 0) {
            // No command to send, send keep-alive
            /*char keepAlive[BUF_SIZE];
            strcpy(keepAlive, "keepalive");
            if (send(sd, keepAlive, sizeof(char)*BUF_SIZE, 0) == -1) {
                printf("Keep-alive failed, connection broken. Exiting\n");
                exit(0);
            }*/
        }
        else {
            // Command ready to send
            int bytesRead = read(0, command, sizeof(char)*BUF_SIZE);
            if (bytesRead >= BUF_SIZE-1) {
                printf("Command too large, exiting.\n");
                exit(1);
            }
            command[bytesRead-1] = '\0';
            if (command[0] == 'p' && command[1] == 'u' && command[2] == 't') {
                putCommand(command, sd);
            }
            else if (strcmp(command, "quit\n") == 0) {
                disconnectFromServer(sd);
                exit(0);
            }
            else {
                send(sd, command, sizeof(char)*BUF_SIZE, 0);
            }
        }
    }
}