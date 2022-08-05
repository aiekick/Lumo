message("------------ Start wxCodeEditor ----------------")
set(CODEEDITOR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/CodeEditor)
set(CODEEDITOR_LIBRARIES ${CODEEDITOR_LIBRARIES} CodeEditor)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/CodeEditor)
set_target_properties(${CODEEDITOR_LIBRARIES} PROPERTIES FOLDER Lumo_Libs)
##set_target_properties(${CODEEDITOR_LIBRARIES} PROPERTIES LINK_FLAGS "/ignore:4244")
message("------------ End wxCodeEditor ----------------")

