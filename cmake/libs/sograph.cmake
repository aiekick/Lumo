set(SOGRAPH_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libs/SoGraph)
set(SOGRAPH_LIBRARIES ${SOGRAPH_LIBRARIES} SoGraph)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/SoGraph)

set_target_properties(SoGraph PROPERTIES FOLDER Lumo_Libs/Static)
##set_target_properties(SoGraph PROPERTIES LINK_FLAGS "/ignore:4244")