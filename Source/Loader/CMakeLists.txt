# ================================================================================================
#  Rekindled Server
#  Copyright (C) 2021 Tim Leonard
#  Copyright (C) 2026 Jake Morgeson
# ================================================================================================

# Prevent errors if configuring this directory standalone (e.g. IDE CMake root = Source/Loader).
# When included via the top-level root CMakeLists, this block is skipped.
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.20)
    project(Loader LANGUAGES C CXX)
endif()

# make sure dotnet is available
find_program(DOTNET_EXECUTABLE dotnet)
if (NOT DOTNET_EXECUTABLE)
    message(FATAL_ERROR "dotnet CLI not found. Please install the .NET SDK and ensure 'dotnet' is available on your PATH to build the Loader project.")
endif()

# expose the Loader project as an external MSBuild target
include_external_msproject(
        Loader
        "${CMAKE_CURRENT_SOURCE_DIR}/Loader.csproj"
)

# override the intermediate output path (default is C:\Intermediate\<plat>_<cfg>)
# so restore/build artifacts live under the CMake binary tree. value must
# be config‑sensitive, hence the $<CONFIG> genexpr. use
# CMAKE_GENERATOR_PLATFORM which exists for all generators (VS maps it from
# CMAKE_VS_PLATFORM_NAME). Fall back to "AnyCPU" when the variable is empty
# to avoid producing a double‑slash in the path.
set(_loader_platform "$<IF:$<BOOL:${CMAKE_GENERATOR_PLATFORM}>,${CMAKE_GENERATOR_PLATFORM},AnyCPU>")

set_property(TARGET Loader PROPERTY
    VS_GLOBAL_BaseIntermediateOutputPath
    "${CMAKE_BINARY_DIR}/Intermediate/${_loader_platform}$<CONFIG>"
)

# ensure MSBuild will automatically restore NuGet packages during build
set_property(TARGET Loader PROPERTY
    VS_GLOBAL_RestorePackagesOnBuild true
)


