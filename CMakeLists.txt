# ================================================================================================
#  DS3OS
#  Copyright (C) 2021 Tim Leonard
# ================================================================================================
cmake_minimum_required(VERSION 3.10)

# Environment settings that all projects inherit.
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Tools/Build/build;${CMAKE_MODULE_PATH}")
set(ENV_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Common configuration for all projects.
include(common)

# OpenSSL's bundled CMakeLists has its own `cmake_minimum_required(3.5)`,
# which triggers a policy error on newer CMake versions when the effective
# policy version exceeds that.  Set a loose policy range to avoid this.
cmake_policy(VERSION 3.10...${CMAKE_VERSION})

project(ds3os C CXX)

# Always generate a compile_commands.json database for tools such as
# the VS Code C/C++ extension.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands for IDEs" FORCE)


add_subdirectory(Source)



