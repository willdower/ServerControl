//
// Created by William on 31/08/2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include "server.h"

#define BUF_SIZE 1025
#define PUT_PORT 9292

void sys(char *buffer, const int socket) {
    pid_t child;

    child = fork();
    if (child != 0) {
        // This is the parent process
        return;
    }
    else {
        // This is the child process
        getOS(buffer);
        send(socket, buffer, sizeof(char)*BUF_SIZE, 0);
        strcpy(buffer, "");
        sprintf(buffer, "Number of Cores: %d", getCores());
        send(socket, buffer, sizeof(char)*BUF_SIZE, 0);
        exit(0);
    }
}

void put(const int socket, const int force, char *progname, const int files, int *sharedMem, int socketLoc) {

    pid_t child;

    child = fork();

    if (child != 0) {
        return;
    }

    char buf[BUF_SIZE], path[BUF_SIZE];
    DIR *dir = opendir(progname);
    if (dir && force == 0) {
        sprintf(buf, "fileexists");
        send(socket, buf, sizeof(char)*BUF_SIZE, 0);
        sharedMem[socketLoc] = 0;
        exit(0);
    }
    else if (dir && force == 1) {
        struct dirent *direntp;
        while ((direntp = readdir(dir)) != NULL) {
            if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0) {
                continue;
            }
            strcpy(path, "");
            sprintf(path, "%s/%s", progname, direntp->d_name);
            if (remove(path) == -1) {
                printf("Failed to remove %s\n", path);
                perror("Reason: ");
            }
        }
    }
    else if (!dir && ENOENT == errno) {
        mkdir(progname, 0777);
    }
    else if (!dir) {
        sprintf(buf, "Failed to open directory for put.\n");
        send(socket, buf, sizeof(char)*BUF_SIZE, 0);
        sharedMem[socketLoc] = 0;
        exit(0);
    }
    closedir(dir);

    sprintf(buf, "proceed");
    send(socket, buf, sizeof(char)*BUF_SIZE, 0);

    for (int i=0;i<files;i++) {
        receiveFile(progname, socket);
    }
    sharedMem[socketLoc] = 0;
    exit(0);
}

void get(const int socket, char *progname, char *filename, int *sharedMem, const int socketLoc) {

    pid_t child;

    child = fork();

    if (child != 0) {
        return;
    }

    char filePath[BUF_SIZE], buf[BUF_SIZE];
    sprintf(filePath, "%s/%s", progname, filename);
    FILE *file = fopen(filePath, "r");
    if (file == NULL && errno == ENOENT) {
        sprintf(buf, "%s not found in %s directory.\n", filename, progname);
        send(socket, buf, sizeof(char)*BUF_SIZE, 0);
        sharedMem[socketLoc] = 0;
        exit(0);
    }
    else if (file == NULL) {
        sprintf(buf, "Could not open %s\n", filename);
        send(socket, buf, sizeof(char)*BUF_SIZE, 0);
        sharedMem[socketLoc] = 0;
        exit(0);
    }
    else {
        send(socket, "proceed", sizeof(char)*BUF_SIZE, 0);
    }

    sendFile(file, socket, filename);
    fclose(file);
    sharedMem[socketLoc] = 0;
    exit(0);
}