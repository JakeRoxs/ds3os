# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================
# Project has been updated to work with newer CMake policies; require at least
# 3.10 to avoid deprecation warnings about compatibility with < 3.10.

cmake_minimum_required(VERSION 3.10)

# Environment settings that all projects inherit.
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Tools/Build/build;${CMAKE_MODULE_PATH}")
set(ENV_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Common configuration for all projects.
include(common)

project(ds3os C CXX)

# Always generate a compile_commands.json database for tools such as
# the VS Code C/C++ extension.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands for IDEs" FORCE)

add_subdirectory(Source)



