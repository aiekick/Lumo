file(GLOB IMGUIZMO_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ImGuizmo/ImGuizmo.h 
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ImGuizmo/ImGuizmo.cpp)
add_library(ImGuizmo STATIC ${IMGUIZMO_FILES})
set_target_properties(ImGuizmo PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ImGuizmo PROPERTIES FOLDER 3rdparty)
set(IMGUIZMO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ImGuizmo)
set(IMGUIZMO_LIBRARIES ImGuizmo)
