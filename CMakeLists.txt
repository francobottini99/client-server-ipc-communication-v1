cmake_minimum_required (VERSION 3.22)

project(IPC)

file(MAKE_DIRECTORY bin)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/Client)
include_directories(${CMAKE_SOURCE_DIR}/include/Server)
include_directories(${CMAKE_SOURCE_DIR}/src/Client)
include_directories(${CMAKE_SOURCE_DIR}/src/Server)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic -Wextra -Wconversion -std=gnu11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(Clients src/Client/Client.c)
add_executable(Server src/Server/Server.c src/Server/ServerUtils.c)

target_link_libraries(Server m)