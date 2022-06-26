set(IMGUI_CANVAS_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui_node_editor/NodeEditor)
set(IMGUI_CANVAS_INCLUDE_DIR ${IMGUI_CANVAS_ROOT_DIR}/Include)
set(IMGUI_CANVAS_LIBRARIES imgui_canvas)

add_library(imgui_canvas STATIC
    ${IMGUI_CANVAS_ROOT_DIR}/Source/imgui_canvas.cpp
    ${IMGUI_CANVAS_ROOT_DIR}/Source/imgui_canvas.h
)

target_include_directories(imgui_canvas PUBLIC ${IMGUI_CANVAS_ROOT_DIR}/Source)
target_link_libraries(imgui_canvas PUBLIC imgui)

set_target_properties(imgui_canvas PROPERTIES FOLDER 3rdparty/nodes)


