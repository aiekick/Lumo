file(GLOB IMGUICOLORTEXTEDIT_FILES 
	${CMAKE_SOURCE_DIR}/3rdparty/ImGuiColorTextEdit/*.h 
	${CMAKE_SOURCE_DIR}/3rdparty/ImGuiColorTextEdit/*.cpp)
add_library(ImGuiColorTextEdit STATIC ${IMGUICOLORTEXTEDIT_FILES})
set_target_properties(ImGuiColorTextEdit PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(ImGuiColorTextEdit PROPERTIES FOLDER 3rdparty)
set(IMGUICOLORTEXTEDIT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/ImGuiColorTextEdit)
set(IMGUICOLORTEXTEDIT_LIBRARIES ImGuiColorTextEdit)
