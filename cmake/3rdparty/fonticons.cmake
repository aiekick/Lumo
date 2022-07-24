set(FONTICONS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/MainLibs/FontIcons)
set(FONTICONS_LIBRARIES ${FONTICONS_LIBRARIES} FontIcons)

add_subdirectory(${CMAKE_SOURCE_DIR}/MainLibs/FontIcons)
set_target_properties(${FONTICONS_LIBRARIES} PROPERTIES FOLDER 3rdparty/main)
##set_target_properties(${FONTICONS_LIBRARIES} PROPERTIES LINK_FLAGS "/ignore:4244")
