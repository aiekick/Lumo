set(CTOOLS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ctools/include)
set(CTOOLS_LIBRARIES ctools)

set(USE_GL_VERSION_CHECKER OFF CACHE BOOL "" FORCE)
set(USE_CONFIG_SYSTEM ON CACHE BOOL "" FORCE)
set(USE_GLFW_CLIPBOARD ON CACHE BOOL "" FORCE)

include_directories(
	${GLFW_INCLUDE_DIR}
)

add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/ctools)

set_target_properties(ctools PROPERTIES FOLDER Lumo_Libs/Shared)
set_target_properties(ctools PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(ctools PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(ctools PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
