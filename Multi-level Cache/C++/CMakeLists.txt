# CMakeLists.txt for project 1 of CS{4/6}290 and ECE{4/6}100 Spring 2019

cmake_minimum_required (VERSION 3.1) # Earlier versions won't support this file

# C++ project
project(cachesim
        DESCRIPTION "Cache hit-miss simulator"
        LANGUAGES CXX)

# Files to submit
# Note -- If you want to specify exactly which pdf file you want to submit change the * to the filename
set(SUBMIT_FILES "${CMAKE_SOURCE_DIR}/cache_driver.cpp"
                 "${CMAKE_SOURCE_DIR}/cache.cpp"
                 "${CMAKE_SOURCE_DIR}/cache.hpp"
                 "${CMAKE_SOURCE_DIR}/CMakeLists.txt"
                 "${CMAKE_SOURCE_DIR}/*.pdf"
                 )

# Sets required flags
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

# Set flags for stricter warnings and errors
if( "${CMAKE_CXX_COMPILER_ID}" MATCHES GNU )
    message(STATUS "Using gcc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wpedantic -Wpointer-arith -Wsign-conversion -Wconversion -pedantic-errors")
elseif( "${CMAKE_CXX_COMPILER_ID}" MATCHES Clang )
    message(STATUS "Using clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wpedantic -Wpointer-arith -Wsign-conversion -Wconversion -pedantic-errors")
endif()

# Enable debugging using gdb or lldb depending on operating system
set (CMAKE_BUILD_TYPE Debug)

# Generate executable
add_executable(cachesim cache_driver.cpp cache.cpp cache.hpp)

set(SUBMIT_DIRECTORY "submit")

# For creating a submittable tar archive
add_custom_target(submit
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SUBMIT_DIRECTORY}
        COMMAND ${CMAKE_COMMAND} -E copy ${SUBMIT_FILES} ${SUBMIT_DIRECTORY}
        COMMAND ${CMAKE_COMMAND} -E tar "cvzf" "project1_submit.tar.gz" ${SUBMIT_DIRECTORY}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SUBMIT_DIRECTORY} )
