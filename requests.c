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
    pid_t child = fork();

    if (child != 0) {
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

    pid_t child = fork();

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

    pid_t child = fork();

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

void run(const int socket, char *recv, int *sharedMem, const int socketLoc) {

    pid_t child = fork();

    if (child != 0) {
        return;
    }

    char progname[BUF_SIZE], args[BUF_SIZE];
    char *save;
    char *token = strtok_r(recv, " ", &save);
    token = strtok_r(NULL, " ", &save);
    strcpy(progname, token);
    token = strtok_r(NULL, " ", &save);
    strcpy(args, "");
    while (token != NULL && strcmp(token, "-f") != 0) {
        strcat(args, token);
        token = strtok_r(NULL, " ", &save);
    }

    char filepath[BUF_SIZE], filename[BUF_SIZE], command[BUF_SIZE];
    strcpy(filename, progname);
    strcat(filename, ".exe");
    sprintf(filepath, "%s/%s", progname, filename);

    FILE *exe = fopen(filepath, "r");
    if (exe == NULL) {
        // Not compiled, make
        makeProgram(progname);
    }
    else {
        fclose(exe);
        struct stat st;
        char latestFile[BUF_SIZE];
        time_t latestChange = 0;
        DIR *dir = opendir(progname);
        struct dirent *direntp;
        while ((direntp = readdir(dir)) != NULL) {
            stat(direntp->d_name, &st);
            if (st.st_mtime > latestChange && strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0) {
                latestChange = st.st_mtime;
                strcpy(latestFile, direntp->d_name);
            }
        }
        if (strcmp(latestFile, filename) != 0) {
            // Exe older than one of the files, make
            makeProgram(progname);
        }
    }

    sprintf(command, "cd %s && ./%s %s", progname, progname, args);
    FILE *p = popen(command, "r");

    FILE *output = fopen("command_out", "w");
    char ch;
    while ((ch = fgetc(p)) != EOF) {
        fputc(ch, output);
    }
    fclose(output);
    output = fopen("command_out", "r");
    sendFile(output, socket, "command_out");
    fclose(output);
    sharedMem[socketLoc] = 0;
    remove("command_out");
    exit(0);
}