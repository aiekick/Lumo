
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/freetype2 EXCLUDE_FROM_ALL)

set_target_properties(freetype PROPERTIES FOLDER 3rdparty)

set(FREETYPE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/freetype2/include)
set(FREETYPE_LIBRARIES freetype)

add_definitions(-DFT_DEBUG_LEVEL_TRACE)