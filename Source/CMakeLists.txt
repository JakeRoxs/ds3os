#== == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == ==
#Rekindled Server
#Copyright(C) 2022 Tim Leonard
#Copyright(C) 2026 Jake Morgeson
#== == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == ==

add_subdirectory(ThirdParty)

option(BUILD_LOADER_AVALONIA "Build the Avalonia loader project" ON)

if (WIN32)
    add_subdirectory(Loader)
    add_subdirectory(Injector)
endif()

if (BUILD_LOADER_AVALONIA)
    add_subdirectory(Loader.Avalonia)
endif()

add_subdirectory(Server)
add_subdirectory(Server.DarkSouls3)
add_subdirectory(Server.DarkSouls2)
add_subdirectory(Shared)
