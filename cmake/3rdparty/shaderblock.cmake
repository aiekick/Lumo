set(SHADERBLOCK_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/MainLibs/ShaderBlock)
set(SHADERBLOCK_LIBRARIES ${SHADERBLOCK_LIBRARIES} ShaderBlock)

add_subdirectory(${SHADERBLOCK_INCLUDE_DIR})
set_target_properties(${SHADERBLOCK_LIBRARIES} PROPERTIES FOLDER MainLibs)

