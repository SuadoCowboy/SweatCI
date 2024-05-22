cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 11)

project(HayBCMD VERSION 0.1.0 LANGUAGES CXX)

option(BUILD_SHARED "Whether build shared or static library." TRUE)

# https://stackoverflow.com/questions/2152077/is-it-possible-to-get-cmake-to-build-both-a-static-and-shared-library-at-the-sam
add_library(objlib OBJECT HayBCMD.cpp)

if(BUILD_SHARED)
    # shared libraries need PIC
    set_property(TARGET objlib PROPERTY POSITION_INDEPENDENT_CODE 1)
    add_library(HayBCMD SHARED $<TARGET_OBJECTS:objlib>)
else()
    add_library(HayBCMD STATIC $<TARGET_OBJECTS:objlib>)
endif()

target_include_directories(HayBCMD PRIVATE ${PROJECT_SOURCE_DIR}) # HayBCMD.h

add_subdirectory(${PROJECT_SOURCE_DIR}/example)