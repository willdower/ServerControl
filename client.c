#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "client.h"

#define BUF_SIZE 1025

int main(int argc, char **argv) {
    fd_set readset, consoleset;
    struct sockaddr_in server;
    int sd = createSocket();
    struct hostent *hp = gethostbyname(argv[1]); // Convert string of hostname to hostname struct

    char command[1025], receive_buf[1025];

    bcopy(hp->h_addr, &(server.sin_addr.s_addr), hp->h_length);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2])); // Convert string of port into int and pass to sin_port

    connectToServer(sd, &server, receive_buf);

    for(;;) {
        FD_ZERO(&readset);
        FD_SET(sd, &readset);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100;
        select(sd+1, &readset, NULL, NULL, &tv);

        if (FD_ISSET(sd, &readset)) {
            // Handle input from server
            read(sd, receive_buf, sizeof(receive_buf));
            printf("%s", receive_buf);
        }

        tv.tv_sec = 0;
        tv.tv_usec = 100;

        FD_ZERO(&consoleset);
        FD_SET(fileno(stdin), &consoleset);
        int num;
        if ((num = select(fileno(stdin)+1, &consoleset, NULL, NULL, &tv)) == 0) {
            // No command to send yet
        }
        else {
            // Command ready to send
            read(0, command, sizeof(command));
            if (strcmp(command, "disconnect\n") == 0) {
                disconnectFromServer(sd);
                exit(0);
            }
            send(sd, command, sizeof(command), 0);
        }
    }
}