cmake_minimum_required(VERSION 3.10)
project(PuneBusRoutePlanner CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(include)

add_executable(route_planner
    src/main.cpp
    src/gtfs_loader.cpp
    src/graph.cpp
    src/pathfinder.cpp
)
