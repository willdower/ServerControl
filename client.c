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
#include <errno.h>
#include "client.h"

#define BUF_SIZE 1025

extern int errno;

void sigpipeHandler(int sig) {
    printf("Connection broken, exiting.\n");
    exit(0);
}

int main(int argc, char **argv) {
    sigaction(SIGPIPE, &(struct sigaction){sigpipeHandler}, NULL);
    sigaction(SIGCHLD, &(struct sigaction){SIG_IGN}, NULL);

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
            if (read(sd, receive_buf, sizeof(receive_buf)) <= 0) {
                printf("Server has closed connection. Exiting.\n");
                exit(0);
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
            fflush(stdin); // Removes previous unwanted input, such as errant keypresses, from stdin
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
                printf("\n");
                putCommand(command, server);
            }
            else if (command[0] == 'g' && command[1] == 'e' && command[2] == 't') {
                printf("\n");
                getCommand(command, sd);
            }
            else if (command[0] == 'r' && command[1] == 'u' && command[2] == 'n') {
                printf("\n");
                runCommand(command, server);
            }
            else if (strcmp(command, "quit") == 0) {
                printf("\n");
                disconnectFromServer(sd);
                exit(0);
            }
            else if (command[0] == 'l' && command[1] == 'i' && command[2] == 's' && command[3] == 't') {
                printf("\n");
                listCommand(command, sd);
            }
            else {
                printf("\n");
                send(sd, command, sizeof(char)*BUF_SIZE, 0);
            }
        }
    }
}