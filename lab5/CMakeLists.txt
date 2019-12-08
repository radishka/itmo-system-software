cmake_minimum_required(VERSION 3.15)
project(lab5 C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(lab5
        task.h
        task10.c
        task1_client.c
        task1_server.c
        task2_client.c
        task2_server.c
        task3_client.c
        task3_server.c
        task4.c
        task5.c
        task6.c
        task7.c
        task8_client.c
        task8_server.c
        task9_server.c)
