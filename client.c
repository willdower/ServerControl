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

void sigpipeHandler(int unusuedVar) {
    printf("Connection broken, exiting.\n");
    exit(0);
}

void getCommand(char *command, const int socket) {
    char response[BUF_SIZE];

    send(socket, command, sizeof(char)*BUF_SIZE, 0);

    read(socket, response, sizeof(char)*BUF_SIZE);
    if (strcmp(response, "proceed") != 0) {
        printf("%s", response);
        return;
    }

    read(socket, response, sizeof(char)*BUF_SIZE);
    char *token = strtok(response, " ");
    int size = atoi(token);
    char *readBuf = (char*)malloc(sizeof(char)*size);
    read(socket, readBuf, size);

    char *ptr, *position = readBuf, *start = readBuf;
    int lines = 0;
    while (1) {
        while ((ptr = strchr(position, '\n')) != NULL && lines <= 40) {
            position = ptr+1;
            lines++;
        }
        if (ptr == NULL) {
            printf("%s", start);
            break;
        }
        else {
            *ptr = '\0';
            printf("%s\n", start);
            *ptr = '\n';
            start = ptr+1;
            lines = 0;
            getchar();
        }
    }
    free(readBuf);
    printf("\n\n\nFinished reading file.\n");
    sprintf(response, "File for 'get' received successfully by client.\n");
    send(socket, response, sizeof(char)*BUF_SIZE, 0);
}

void putCommand(char *command, struct sockaddr_in server) {

    char buf[BUF_SIZE];

    pid_t child = fork();

    if (child != 0) {
        return;
    }

    int socket = createSocket();

    connectToServer(socket, server, command);

    read(socket, buf, sizeof(char)*BUF_SIZE); // Get the welcome message

    int force = 0, progDone = 0, files = 0;
    char original_command[BUF_SIZE], progname[BUF_SIZE];
    strcpy(original_command, command);
    char *token = strtok(command, " ");
    token = strtok(NULL, " ");
    while (token != NULL) {
        if (strcmp(token, "-f") == 0) {
            force = 1;
        }
        else if (progDone == 0) {
            strcpy(progname, token);
            progDone = 1;
        }
        else {
            FILE *test;
            if ((test = fopen(token, "r")) == NULL) {
                if (errno == ENOENT) {
                    printf("File %s not found.\n", token);
                    printf("No files have been uploaded.\n");
                    fclose(test);
                    exit(0);
                }
                else {
                    printf("File %s cannot be uploaded.\n", token);
                    perror("Reason: ");
                    printf("No files have been uploaded.\n");
                    fclose(test);
                    exit(0);
                }
            }
            else {
                files++;
            }
            fclose(test);
        }
        token = strtok(NULL, " ");
    }

    // Start put command with server, specifying whether to force, the progname, and the number of files being sent
    sprintf(buf, "put %d %s %d", force, progname, files);
    send(socket, buf, sizeof(char)*BUF_SIZE, 0);

    read(socket, command, sizeof(char)*BUF_SIZE);
    if (strcmp(command, "fileexists") == 0) {
        printf("%s directory already exists, please try again with -f to force.\n", progname);
        fflush(stdout);
        exit(0);
    }
    else if (strcmp(command, "proceed") != 0) {
        printf("Server has halted put command. Reason: ");
        printf("%s\n", command);
        fflush(stdout);
        exit(0);
    }

    strcpy(command, original_command);

    char *savePtr;

    token = strtok_r(original_command, " ", &savePtr);
    token = strtok_r(NULL, " ", &savePtr);

    int atLeastOneFile = 0;
    progDone = 0;
    while (token != NULL) {
        if (strcmp(token, "-f") == 0) {
        }
        else if (progDone == 0) {
            progDone = 1;
        }
        else {
            atLeastOneFile = 1;
            FILE *file = fopen(token, "r");
            char filename[BUF_SIZE];
            strcpy(filename, token);
            getFilenameFromPath(filename);
            sendFile(file, socket, filename);
            fclose(file);
        }
        token = strtok_r(NULL, " ", &savePtr);
    }

    if (progDone == 0) {
        printf("Incorrect syntax, progname not found.\n");
        fflush(stdout);
    }
    if (progDone == 1 && atLeastOneFile == 0) {
        printf("Incorrect syntax, no filenames found.\n");
    }

    close(socket);
    exit(0);
}

void runCommand(char *command, struct sockaddr_in server) {

    pid_t child = fork();

    if (child != 0) {
        return;
    }

    char buf[BUF_SIZE];

    int socket = createSocket();

    connectToServer(socket, server, command);

    read(socket, buf, sizeof(char)*BUF_SIZE); // Get the welcome message
    strcpy(buf, "");

    char *token, *ptr;
    FILE *output = NULL;
    token = strtok_r(command, " ", &ptr);
    while (token != NULL) {
        if (strcmp(token, "-f") == 0) {
            token = strtok_r(NULL, " ", &ptr);
            output = fopen(token, "w");
        }
        else {
            strcat(buf, token);
            strcat(buf, " ");
        }
        token = strtok_r(NULL, " ", &ptr);
    }
    if (output == NULL) {
        output = stdout;
    }

    send(socket, buf, sizeof(char)*BUF_SIZE, 0);

    receiveOnClient(output, socket);
    close(socket);
    exit(0);
}

void listCommand(char *command, const int socket) {
    char response[BUF_SIZE];
    send(socket, command, sizeof(char)*BUF_SIZE, 0);
    read(socket, response, sizeof(char)*BUF_SIZE);
    while (strcmp(response, "complete") != 0) {
        printf("%s", response);
        read(socket, response, sizeof(char)*BUF_SIZE);
    }
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
            fflush(stdin);
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
                putCommand(command, server);
            }
            else if (command[0] == 'g' && command[1] == 'e' && command[2] == 't') {
                getCommand(command, sd);
            }
            else if (command[0] == 'r' && command[1] == 'u' && command[2] == 'n') {
                runCommand(command, server);
            }
            else if (strcmp(command, "quit") == 0) {
                disconnectFromServer(sd);
                exit(0);
            }
            else if (command[0] == 'l' && command[1] == 'i' && command[2] == 's' && command[3] == 't') {
                listCommand(command, sd);
            }
            else {
                send(sd, command, sizeof(char)*BUF_SIZE, 0);
            }
        }
    }
}