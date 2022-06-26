file(GLOB IMGUI_CUSTOM_WIDGETS ${CMAKE_SOURCE_DIR}/mainlibs/ImGuiCustomWidgets/*.*)
source_group(src FILES ${IMGUI_CUSTOM_WIDGETS})

add_library(ImGuiCustomWidgets STATIC ${IMGUI_CUSTOM_WIDGETS})

include_directories(
	${IMGUI_INCLUDE_DIR}
)

set_target_properties(ImGuiCustomWidgets PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ImGuiCustomWidgets PROPERTIES FOLDER MainLibs)

set(IMGUI_CUSTOM_WIDGETS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/mainlibs/ImGuiCustomWidgets)
set(IMGUI_CUSTOM_WIDGETS_LIBRARIES ImGuiCustomWidgets)

