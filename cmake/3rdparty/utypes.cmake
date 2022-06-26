set(UTYPES_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/MainLibs/uTypes)
set(UTYPES_LIBRARIES ${UTYPES_LIBRARIES} uTypes)

add_subdirectory(${CMAKE_SOURCE_DIR}/MainLibs/uTypes)
set_target_properties(${UTYPES_LIBRARIES} PROPERTIES FOLDER MainLibs)
##set_target_properties(${UTYPES_LIBRARIES} PROPERTIES LINK_FLAGS "/ignore:4244")
