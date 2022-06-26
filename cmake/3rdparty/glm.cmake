add_definitions(-DGLM_FORCE_SILENT_WARNINGS)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/glm)

set_target_properties(uninstall PROPERTIES FOLDER CMakePredefinedTargets)

set(GLM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/glm/glm)
set(GLM_LIBRARIES ${GLM_LIBRARIES} glm)
