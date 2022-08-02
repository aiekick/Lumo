set(CODETREE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/MainLibs/CodeTree)
set(CODETREE_LIBRARIES ${CODETREE_LIBRARIES} CodeTree)

add_subdirectory(${CMAKE_SOURCE_DIR}/MainLibs/CodeTree)
set_target_properties(${CODETREE_LIBRARIES} PROPERTIES FOLDER MainLibs)
##set_target_properties(${CODETREE_LIBRARIES} PROPERTIES LINK_FLAGS "/ignore:4244")

