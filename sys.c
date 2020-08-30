//
// Created by William on 30/08/2020.
//
#include <string.h>
#include <stdlib.h>
#include <windows.h>

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