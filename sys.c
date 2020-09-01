//
// Created by William on 30/08/2020.
//
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include "client.h"

#define BUF_SIZE 1025

void *allocateSharedMemory(size_t size) {
    int prot = PROT_READ | PROT_WRITE;
    int vis = MAP_SHARED | MAP_ANONYMOUS;
    return mmap(NULL, size, prot, vis, -1, 0);
}

void getOS(char *string) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #ifdef _WIN64
        strcpy(string, "OS: Windows 64-bit");
    #else
        strcpy(string, "OS: Windows 32-bit");
    #endif
#elif __APPLE__ || __MACH__
    strcpy(string, "OS: Mac OS X");
#elif __linux__
    strcpy(string, "OS: Linux");
#elif __FreeBSD__
    strcpy(string, "OS: FreeBSD");
#elif __unix__ || __unix
    strcpy(string, "OS: Unix");
#else
    strcpy(string, "OS: Unknown OS");
#endif
}

int getCores() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    return siSysInfo.dwNumberOfProcessors;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

long getFileSize(char *filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;;
}

void sendFile(FILE *file, const int socket, const int force, char *progname, char *filename) {
    char command[BUF_SIZE];
    sprintf(command, "put %d %s %s", force, progname, filename);
    send(socket, command, sizeof(char)*BUF_SIZE, 0);

    read(socket, command, sizeof(char)*BUF_SIZE);
    if (strcmp(command, "fileexists") == 0) {
        printf("File already exists, please try again with -f to force.\n");
        fflush(stdout);
        return;
    }
    else if (strcmp(command, "proceed") != 0) {
        printf("Server has halted put command. Reason: ");
        printf("%s\n", command);
        fflush(stdout);
        return;
    }

    long fileSize = getFileSize(filename);
    char *buf = (char*)malloc(sizeof(char)*fileSize);
    fread(buf, sizeof(char), fileSize, file);
    char sizeBuf[BUF_SIZE];
    sprintf(sizeBuf, "%ld", fileSize);
    send(socket, sizeBuf, sizeof(char)*BUF_SIZE, 0);
    send(socket, buf, sizeof(char)*fileSize, 0);
    free(buf);
    read(socket, command, sizeof(char)*BUF_SIZE);
    printf("%s", command);
}

void receiveFile(FILE *file, const int socket) {
    char sizeBuf[BUF_SIZE];
    strcpy(sizeBuf, "");
    read(socket, sizeBuf, sizeof(char)*BUF_SIZE);
    unsigned long fileSize = atoi(sizeBuf);
    char *buf = malloc(sizeof(char)*fileSize);
    read(socket, buf, fileSize);
    //fprintf(file, "%s", buf);
    for (int i=0;i<fileSize;i++) {
        fputc(buf[i], file);
    }
    free(buf);
    char command[BUF_SIZE];
    sprintf(command, "File successfully put.\n");
    send(socket, command, sizeof(char)*BUF_SIZE, 0);
}