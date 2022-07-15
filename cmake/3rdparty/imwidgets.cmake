set(IMWIDGETS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/MainLibs/ImWidgets)
set(IMWIDGETS_LIBRARIES ${UTYPES_LIBRARIES} ImWidgets)

add_subdirectory(${CMAKE_SOURCE_DIR}/MainLibs/ImWidgets)
set_target_properties(${IMWIDGETS_LIBRARIES} PROPERTIES FOLDER MainLibs)
##set_target_properties(${IMWIDGETS_LIBRARIES} PROPERTIES LINK_FLAGS "/ignore:4244")

