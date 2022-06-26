add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/zep/src)

set_target_properties(Zep PROPERTIES FOLDER 3rdparty)

set(ZEP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/zep/include)
set(ZEP_LIBRARIES Zep)

