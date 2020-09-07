//
// Created by William on 30/08/2020.
//
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

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

void getFilenameFromPath(char *path) {
    char filename[BUF_SIZE];
    char *token = strtok(path, "/");
    while (token != NULL) {
        strcpy(filename, token);
        token = strtok(NULL, "/");
    }
    strcpy(path, filename);
}

long getFileSize(char *filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;;
}

void sendFile(FILE *file, const int socket, char *filename) {
    char command[BUF_SIZE];

    long fileSize = getFileSize(filename);
    char *buf = (char*)malloc(sizeof(char)*fileSize);
    fread(buf, sizeof(char), fileSize, file);
    char sizeBuf[BUF_SIZE];
    sprintf(sizeBuf, "%ld %s", fileSize, filename);
    send(socket, sizeBuf, sizeof(char)*BUF_SIZE, 0);
    send(socket, buf, sizeof(char)*fileSize, 0);
    free(buf);
    read(socket, command, sizeof(char)*BUF_SIZE);
    printf("%s", command);
}

void receiveFile(char *progname, const int socket) {
    char dataBuf[BUF_SIZE], command[BUF_SIZE];
    strcpy(dataBuf, "");
    read(socket, dataBuf, sizeof(char)*BUF_SIZE);
    char *token = strtok(dataBuf, " ");
    unsigned long fileSize = atoi(token);
    token = strtok(NULL, " ");

    char filePath[BUF_SIZE];
    sprintf(filePath, "%s/%s", progname, token);

    if (strcmp(token, ".") == 0 || strcmp(token, "/") == 0) {
        sprintf(command, "Illegal filename.\n");
        send(socket, command, sizeof(char)*BUF_SIZE, 0);
        return;
    }


    FILE *file = fopen(filePath, "w");
    char *buf = malloc(sizeof(char)*fileSize);
    if (fileSize > 0) {
        read(socket, buf, fileSize);
        for (int i=0;i<fileSize;i++) {
            fputc(buf[i], file);
        }
    }
    free(buf);
    sprintf(command, "File %s successfully put.\n", token);
    send(socket, command, sizeof(char)*BUF_SIZE, 0);
    fclose(file);
}

void receiveOnClient(FILE *file, const int socket) {
    char dataBuf[BUF_SIZE], command[BUF_SIZE];
    strcpy(dataBuf, "");
    read(socket, dataBuf, sizeof(char)*BUF_SIZE);
    char *token = strtok(dataBuf, " ");
    unsigned long fileSize = atoi(token);

    char *buf = malloc(sizeof(char)*fileSize);
    if (fileSize > 0) {
        read(socket, buf, fileSize);
        for (int i=0;i<fileSize;i++) {
            fputc(buf[i], file);
        }
    }
    free(buf);
    fclose(file);
}

void makeProgram(char *progname) {
    char filepath[BUF_SIZE], command[BUF_SIZE];
    printf("Compiling %s...\n", progname);
    sprintf(filepath, "%s/GNUmakefile", progname);
    FILE *makefile = fopen(filepath, "r");
    if (makefile == NULL) {
        sprintf(filepath, "%s/GNUmakefile", progname);
        makefile = fopen(filepath, "r");
        if (makefile == NULL) {
            sprintf(filepath, "%s/Makefile", progname);
            makefile = fopen(filepath, "r");
            if (makefile == NULL) {
                sprintf(filepath, "%s/makefile", progname);
                makefile = fopen(filepath, "r");
                if (makefile == NULL) {
                    printf("No makefile found, using default makefile.\n");
                    sprintf(command, "cd %s && make -f ../default_makefile", progname);
                    printf("\nOutput of make:\n");
                    FILE *p = popen(command, "r");
                    char ch;
                    while ((ch = fgetc(p)) != EOF) {
                        fputc(ch, stdout);
                    }
                }
            }
            else {
                fclose(makefile);
                printf("\nOutput of make:\n");
                sprintf(command, "cd %s && make -f makefile", progname);
                popen(command, "r");
            }
        }
        else {
            fclose(makefile);
            printf("\nOutput of make:\n");
            sprintf(command, "cd %s && make -f Makefile", progname);
            popen(command, "r");
        }
    }
    else {
        fclose(makefile);
        printf("\nOutput of make:\n\n");
        sprintf(command, "cd %s && make -f GNUmakefile", progname);
        popen(command, "r");
    }
}