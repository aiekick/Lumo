add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/SPIRV-Headers)
##set_target_properties(SPIRV-Headers PROPERTIES FOLDER 3rdparty)

set(SPIRV_HEADERS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/SPIRV-Headers/include)
set(SPIRV_HEADERS_LIBRARIES spirv-headers)