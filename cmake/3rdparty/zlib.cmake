add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/zlib)

set(ZLIB_HOME ${CMAKE_SOURCE_DIR}/3rdparty/zlib)
set(ENV_ZLIB_HOME ${CMAKE_SOURCE_DIR}/3rdparty/zlib)
set(ZLIB_LIBRARIES zlibstatic)
set(ZLIB_LIBRARY zlibstatic)
set(ZLIB_LIBRARY_REL zlibstatic)
set(ZLIB_LIBRARY_DBG zlibstatic)
set(ZLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/zlib)

set_target_properties(zlibstatic PROPERTIES FOLDER 3rdparty)
set_target_properties(zlib PROPERTIES FOLDER 3rdparty)
