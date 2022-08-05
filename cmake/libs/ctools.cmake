set(USE_GL_VERSION_CHECKER OFF CACHE BOOL "" FORCE)
set(USE_CONFIG_SYSTEM ON CACHE BOOL "" FORCE)
set(USE_GLFW_CLIPBOARD ON CACHE BOOL "" FORCE)

include_directories(
	${GLFW_INCLUDE_DIR}
)

add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/ctools)

set_target_properties(ctools PROPERTIES FOLDER Lumo_Libs/tools)
set_target_properties(ctools PROPERTIES LINK_FLAGS "/ignore:4244")