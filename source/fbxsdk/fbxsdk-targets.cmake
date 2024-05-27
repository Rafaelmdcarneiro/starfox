# Include this file in CMakeLists.txt and link against it like this:
#
# include(${CMAKE_CURRENT_LIST_DIR}/../fbxsdk/fbxsdk-targets.cmake)
# target_link_libraries(mytarget PRIVATE fbxsdk)


# Find FBX SDK and an create import target "fbxsdk" so that child CMake scripts can target_link_libraries(<target> fbxsdk)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(FBX REQUIRED)
add_library(fbxsdk-libs UNKNOWN IMPORTED)
set_target_properties(fbxsdk-libs PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${FBX_INCLUDE_DIRS})
set_target_properties(fbxsdk-libs PROPERTIES IMPORTED_LOCATION_DEBUG ${FBX_LIBRARIES_DEBUG})
set_target_properties(fbxsdk-libs PROPERTIES IMPORTED_LOCATION_RELEASE ${FBX_LIBRARIES})
set_target_properties(fbxsdk-libs PROPERTIES IMPORTED_LOCATION_MINSIZEREL ${FBX_LIBRARIES})
set_target_properties(fbxsdk-libs PROPERTIES IMPORTED_LOCATION_RELWITHDEBINFO ${FBX_LIBRARIES})

add_library(fbxsdk INTERFACE IMPORTED)
set_target_properties(fbxsdk PROPERTIES INTERFACE_LINK_LIBRARIES fbxsdk-libs)
