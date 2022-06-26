add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/SPIRV-Tools)
set_target_properties(SPIRV-Tools PROPERTIES FOLDER 3rdparty)

set(SPIRV_TOOLS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/SPIRV-Tools/include)
set(SPIRV_TOOLS_LIBRARIES SPIRV-Tools SPIRV-Tools-opt SPIRV-Tools SPIRV-Tools-link)