# ================================================================================================
#  Rekindled Server
#  Copyright (C) 2021 Tim Leonard
#  Copyright (C) 2026 Jake Morgeson
# ================================================================================================
cmake_minimum_required(VERSION 3.20)

project(rekindled-server C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands for IDEs" FORCE)

if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
    message(FATAL_ERROR "Use Ninja generator (-G Ninja).")
endif()

add_subdirectory(Source)



