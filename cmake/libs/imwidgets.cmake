set(IMWIDGETS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/MainLibs/ImWidgets)
set(IMWIDGETS_LIBRARIES ${IMWIDGETS_LIBRARIES} ImWidgets)

add_subdirectory(${CMAKE_SOURCE_DIR}/MainLibs/ImWidgets)
set_target_properties(${IMWIDGETS_LIBRARIES} PROPERTIES FOLDER libs/ui)
##set_target_properties(${IMWIDGETS_LIBRARIES} PROPERTIES LINK_FLAGS "/ignore:4244")

