cmake_minimum_required(VERSION 3.17)
project(SDCAssignment1 C)

set(CMAKE_C_STANDARD 11)

add_executable(SDCAssignment1-Client client.c server.c)
add_executable(SDCAssignment1-Server server.c networking.c server.h)