set(UTYPES_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/uTypes)
set(UTYPES_LIBRARIES ${UTYPES_LIBRARIES} uTypes)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/uTypes)
set_target_properties(${UTYPES_LIBRARIES} PROPERTIES FOLDER Lumo_Libs/Static)
##set_target_properties(${UTYPES_LIBRARIES} PROPERTIES LINK_FLAGS "/ignore:4244")
