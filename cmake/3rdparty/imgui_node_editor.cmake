set(IMGUI_NODE_EDITOR_ROOT_DIR ${CMAKE_SOURCE_DIR}/3rdparty/imgui_node_editor/NodeEditor)
set(IMGUI_NODE_EDITOR_INCLUDE_DIR ${IMGUI_NODE_EDITOR_ROOT_DIR}/Include)
set(IMGUI_NODE_EDITOR_LIBRARIES ${IMGUI_NODE_EDITOR_LIBRARIES} imgui_node_editor)

add_library(imgui_canvas STATIC
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_canvas.cpp
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_canvas.h
)

target_include_directories(imgui_canvas PUBLIC ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source)
target_link_libraries(imgui_canvas PUBLIC imgui)

set(_imgui_node_editor_Canvas
	${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_canvas.cpp 
	${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_canvas.h)
source_group("canvas" FILES ${_imgui_node_editor_Canvas})

set(_imgui_node_editor_Sources
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Include/imgui_node_editor.h
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/crude_json.cpp
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/crude_json.h
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_bezier_math.h
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_bezier_math.inl
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_extra_math.h
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_extra_math.inl
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_node_editor_api.cpp
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_node_editor_internal.h
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_node_editor_internal.inl
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source/imgui_node_editor.cpp
    ${IMGUI_NODE_EDITOR_ROOT_DIR}/Support/imgui_node_editor.natvis
)
source_group("src" FILES ${_imgui_node_editor_Sources})

add_library(imgui_node_editor STATIC 
	${_imgui_node_editor_Sources}
	${_imgui_node_editor_Canvas})

set_property(TARGET imgui_node_editor PROPERTY FOLDER "NodeEditor")

target_link_libraries(imgui_node_editor PUBLIC imgui)

target_include_directories(imgui_node_editor PUBLIC  ${IMGUI_NODE_EDITOR_ROOT_DIR}/Include)
target_include_directories(imgui_node_editor PRIVATE ${IMGUI_NODE_EDITOR_ROOT_DIR}/Source)

set_target_properties(imgui_node_editor PROPERTIES FOLDER 3rdparty/nodes)
set_target_properties(imgui_canvas PROPERTIES FOLDER 3rdparty/nodes)


