cmake_minimum_required(VERSION 3.12)
project(6lw C)

set(CMAKE_C_STANDARD 99)

set (CMAKE_C_FLAGS "-pthread")
add_executable(server server.c queue.c)
add_executable(client client.c queue.c)