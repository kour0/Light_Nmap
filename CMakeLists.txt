cmake_minimum_required(VERSION 3.27)
project(Nmap C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(server
        ping.c
        ping.h
        server.c
        commands.c
        commands.h
        scan_ip.c
        scan_ip.h
        help.c
        help.h
        scan_port.c
        scan_port.h)

add_executable(client
        client.c)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)
