cmake_minimum_required(VERSION 3.17)
project(SDCAssignment1 C)

set(CMAKE_C_STANDARD 11)

add_executable(SDCAssignment1-Client client.c networking.c sys.c client.h)
add_executable(SDCAssignment1-Server server.c networking.c server.h sys.c requests.c)