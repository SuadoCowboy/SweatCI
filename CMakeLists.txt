cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

project(HayBCMD VERSION 0.2.0 LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# https://stackoverflow.com/questions/2152077/is-it-possible-to-get-cmake-to-build-both-a-static-and-shared-library-at-the-sam
add_library(objlib OBJECT HayBCMD.cpp)

if(BUILD_SHARED_LIBS)
    add_library(HayBCMD SHARED $<TARGET_OBJECTS:objlib>)
    set_target_properties(HayBCMD PROPERTIES VERSION ${PROJECT_VERSION} POSITION_INDEPENDENT_CODE 1)
else()
    add_library(HayBCMD STATIC $<TARGET_OBJECTS:objlib>)
endif()

target_include_directories(HayBCMD PRIVATE ${PROJECT_SOURCE_DIR}) # HayBCMD.h

add_subdirectory(${PROJECT_SOURCE_DIR}/example)