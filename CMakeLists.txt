cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

project(SweatCI VERSION 0.3.0 LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

if(BUILD_SHARED_LIBS)
    add_library(SweatCI SHARED SweatCI.cpp)
    set_target_properties(SweatCI PROPERTIES VERSION ${PROJECT_VERSION} POSITION_INDEPENDENT_CODE 1)
else()
    add_library(SweatCI STATIC SweatCI.cpp)
endif()

target_include_directories(SweatCI PRIVATE ${PROJECT_SOURCE_DIR}) # SweatCI.h

add_subdirectory(${PROJECT_SOURCE_DIR}/example)
