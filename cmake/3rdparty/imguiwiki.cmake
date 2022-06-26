file(GLOB PROJECT_IMGUI_WIKI ${CMAKE_SOURCE_DIR}/mainlibs/ImGuiWiki/*.*)
source_group(src FILES ${PROJECT_IMGUI_WIKI})

add_library(ImGuiWiki STATIC 
	${PROJECT_IMGUI_WIKI}
)

include_directories(
	${IMGUI_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/mainlibs
)

set_target_properties(ImGuiWiki PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ImGuiWiki PROPERTIES FOLDER MainLibs)

set(IMGUI_WIKI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/mainlibs/ImGuiWiki)
set(IMGUI_WIKI_LIBRARIES ImGuiWiki)

