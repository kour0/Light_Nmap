cmake_minimum_required(VERSION 3.27)
project(Nmap C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(server
        ping.c
        ping.h
        server.c
        commands.c
        commands.h)

add_executable(client
        client.c)
