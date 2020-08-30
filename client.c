#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char **argv) {
    fd_set readset, consoleset;
    struct sockaddr_in server;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *hp = gethostbyname(argv[1]);

    char command[1024], buf[1024];

    bcopy(hp->h_addr, &(server.sin_addr.s_addr), hp->h_length);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));

    connect(sd, (struct sockaddr*)&server, sizeof(server));
    recv(sd, buf, sizeof(buf), 0);
    printf("%s", buf);

    for(;;) {
        FD_ZERO(&readset);
        FD_SET(sd, &readset);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100;
        select(sd+1, &readset, NULL, NULL, &tv);

        if (FD_ISSET(sd, &readset)) {
            read(sd, buf, sizeof(buf));
            printf("%s", buf);
        }

        tv.tv_sec = 0;
        tv.tv_usec = 100;

        FD_ZERO(&consoleset);
        FD_SET(fileno(stdin), &consoleset);
        int num;
        if ((num = select(fileno(stdin)+1, &consoleset, NULL, NULL, &tv)) == 0) {
            // No entry in one second
        }
        else {
            read(0, command, sizeof(command));
            send(sd, command, sizeof(command), 0);
        }
    }
}