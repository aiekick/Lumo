set(FONTICONS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/FontIcons)
set(FONTICONS_LIBRARIES ${FONTICONS_LIBRARIES} FontIcons)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/FontIcons)
set_target_properties(${FONTICONS_LIBRARIES} PROPERTIES FOLDER Lumo_Libs/Static/ui)
##set_target_properties(${FONTICONS_LIBRARIES} PROPERTIES LINK_FLAGS "/ignore:4244")
