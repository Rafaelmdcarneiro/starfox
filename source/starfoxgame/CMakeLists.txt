cmake_minimum_required (VERSION 3.2)
project (starfoxgame)

file(GLOB SRC "src/*.cpp" "src/*.h")
add_executable(starfoxgame ${SRC})

# gsgamelib
add_subdirectory(../gsgamelib ../gsgamelib)
target_link_libraries(starfoxgame PRIVATE gsgamelib)
if (BUILD_SHARED_LIBS)
	add_custom_command(TARGET starfoxgame POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:gsgamelib>" $<TARGET_FILE_DIR:starfoxgame>)
endif()

# fbx sdk
include(${CMAKE_CURRENT_LIST_DIR}/../fbxsdk/fbxsdk-targets.cmake)
target_link_libraries(starfoxgame PRIVATE fbxsdk)

# install rules
# e.g.: cmake -DCMAKE_INSTALL_PREFIX=..
install(TARGETS starfoxgame	RUNTIME DESTINATION bin)
if (BUILD_SHARED_LIBS)
	install(FILES $<TARGET_FILE:gsgamelib> DESTINATION bin)
endif()
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION bin)
