//
// Created by William on 30/08/2020.
//
#include <string.h>

void getOS(char *string) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #ifdef _WIN64
        strcpy(string, "Windows 64-bit\0");
    #else
        strcpy(string, "Windows 32-bit\0");
    #endif
#elif __APPLE__ || __MACH__
    strcpy(string, "Mac OS X\0");
#elif __linux__
    strcpy(string, "Linux\0");
#elif __FreeBSD__
    strcpy(string, "FreeBSD\0");
#elif __unix__ || __unix
    strcpy(string, "Unix\0");
#else
    strcpy(string, "Unknown OS\0");
#endif
}