# CMake rules for Shared unit tests

# The test executable verifies generic utilities from Shared/Game.
# It uses minimal custom types so no extra dependencies are required.

# Ensure CTest is enabled so "add_test" calls take effect.
enable_testing()

add_executable(SharedGameUtilsTests
    PlayerDataUtilsTests.cpp
)

target_include_directories(SharedGameUtilsTests PUBLIC
    "${CMAKE_SOURCE_DIR}/Source/Server"
)

target_link_libraries(SharedGameUtilsTests
    Shared
)

# Register with CTest so it runs with "ctest" or "Build_CMakeTools" tests
add_test(NAME SharedGameUtilsTests COMMAND SharedGameUtilsTests)
