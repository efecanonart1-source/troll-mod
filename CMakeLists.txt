cmake_minimum_required(VERSION 3.21)

# Change to whatever your mod is called
project(TrollMode VERSION 1.0.0)

# If you're using C++20/23 features, you'll want to set this
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.hpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# If you are not using geode-cli to build your mod
# you need to call this to setup required build steps
if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode SDK root directory.")
else()
    message(STATUS "Found Geode SDK: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})
