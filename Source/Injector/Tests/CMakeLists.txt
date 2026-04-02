#Tests for Injector components

#Ensure CTest is enabled so tests are discoverable.
enable_testing()

    add_executable(InjectorTests
                       InjectorTestsMain.cpp
                           InjectorInitTests.cpp
                               HookManagerTests.cpp)

        target_link_libraries(InjectorTests PUBLIC InjectorCore Shared detours)

#Register with CTest so it runs with "ctest" or the CMakeTools test runner
            add_test(NAME InjectorTests COMMAND InjectorTests)

                target_include_directories(InjectorTests PUBLIC
                                           "${CMAKE_SOURCE_DIR}/Source/Injector")
