cmake_minimum_required(VERSION 3.24)
project(ServerMonitoring)

file(GLOB_RECURSE SOURCE_FILES src/*.hpp src/*.cpp)

set(CMAKE_CXX_STANDARD 20)

add_executable(ServerMonitoring ${SOURCE_FILES})
