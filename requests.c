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

    /**pid_t child = fork();

    if (child != 0) {
        return;
    }**/

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
    //exit(0);
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

void list(char *command, const int socket) {

    pid_t child = fork();

    if (child != 0) {
        return;
    }

    int l = 0, progDone = 0;
    char progname[BUF_SIZE];
    strcpy(progname, "");
    char *token = strtok(command, " ");
    token = strtok(NULL, " ");
    while (token != NULL) {
        if (strcmp(token, "-l") == 0) {
            l = 1;
        }
        else if (progDone == 0) {
            strcpy(progname, token);
            progDone = 1;
        }
        token = strtok(NULL, " ");
    }

    struct stat fileStatus;
    DIR *dir_ptr;
    struct dirent *direntp;
    if (strcmp(progname, "") == 0) {
        if ((dir_ptr = opendir(".")) == NULL) {
            printf("Open directory failed.");
            exit(2);
        }
    }
    else {
        if ((dir_ptr = opendir(progname)) == NULL) {
            printf("Open directory failed.");
            exit(2);
        }
    }

    char line[BUF_SIZE], addition[BUF_SIZE];
    while ((direntp = readdir(dir_ptr)) != NULL) {
        stat(direntp->d_name, &fileStatus);
        if (strcmp(progname , "") == 0 && !S_ISDIR(fileStatus.st_mode)) {
            continue;
        }
        if (l == 0 && strcmp(direntp->d_name, ".") == 0) {
            continue;
        }
        if (l == 0 && strcmp(direntp->d_name, "..") == 0) {
            continue;
        }
        strcpy(line, "");
        if (l == 1) {
            // File type
            if (S_ISREG(fileStatus.st_mode)) {
                strcat(line, "-");
            } else if (S_ISDIR(fileStatus.st_mode)) {
                strcat(line, "d");
            } else if (S_ISBLK(fileStatus.st_mode)) {
                strcat(line, "b");
            } else if (S_ISCHR(fileStatus.st_mode)) {
                strcat(line, "c");
            } else if (S_ISFIFO(fileStatus.st_mode)) {
                strcat(line, "p");
            } else if (S_ISLNK(fileStatus.st_mode)) {
                strcat(line, "l");
            } else if (S_ISSOCK(fileStatus.st_mode)) {
                strcat(line, "s");
            } else {
                strcat(line, "x");
            }

            // Permissions
            if (fileStatus.st_mode & 256) {
                strcat(line, "r");
            } else {
                strcat(line, "-");
            }
            if (fileStatus.st_mode & 128) {
                strcat(line, "w");
            } else {
                strcat(line, "-");
            }
            if (fileStatus.st_mode & 64) {
                strcat(line, "x");
            } else {
                strcat(line, "-");
            }
            if (fileStatus.st_mode & 32) {
                strcat(line, "r");
            } else {
                strcat(line, "-");
            }
            if (fileStatus.st_mode & 16) {
                strcat(line, "w");
            } else {
                strcat(line, "-");
            }
            if (fileStatus.st_mode & 8) {
                strcat(line, "x");
            } else {
                strcat(line, "-");
            }
            if (fileStatus.st_mode & 4) {
                strcat(line, "r");
            } else {
                strcat(line, "-");
            }
            if (fileStatus.st_mode & 2) {
                strcat(line, "w");
            } else {
                strcat(line, "-");
            }
            if (fileStatus.st_mode & 1) {
                strcat(line, "x");
            } else {
                strcat(line, "-");
            }

            strcat(line, " ");

            // Owner user and group
            sprintf(addition, "%d", fileStatus.st_uid);
            strcat(line, addition);
            strcat(line, " ");
            sprintf(addition, "%d", fileStatus.st_gid);
            strcat(line, addition);
            strcat(line, " ");

            // Size
            sprintf(addition, "%ld", fileStatus.st_size);
            strcat(line, addition);
            strcat(line, " ");

            // Modification time
            time_t modTime = fileStatus.st_mtime;
            strftime(addition, sizeof(char)*BUF_SIZE, "%b %d %Y %H:%M", localtime(&modTime));
            strcat(line, addition);
            strcat(line, " ");
        }
        sprintf(addition, "%s\n", direntp->d_name);
        strcat(line, addition);

        send(socket, line, sizeof(char)*BUF_SIZE, 0);
    }
    closedir(dir_ptr);
    send(socket, "complete", sizeof(char)*BUF_SIZE, 0);
}
