cmake_minimum_required (VERSION 3.2)
project (starfox)

# Enable to build DLLs instead of static libs
set(BUILD_SHARED_LIBS Off)

# Set install dir to current directory (game will end up in 'bin' folder)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(source/starfoxgame)
