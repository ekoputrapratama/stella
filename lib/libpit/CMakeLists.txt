cmake_minimum_required(VERSION 3.15)
project(libpit VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(LIBPIT_HEADER_FILES
    libpit.h)
set(LIBPIT_SOURCE_FILES
    libpit.cpp)

add_library(libpit STATIC ${LIBPIT_SOURCE_FILES} ${LIBPIT_HEADER_FILES})
