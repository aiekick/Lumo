set(VKPROFILER_INCLUDE_DIR)
set(VKPROFILER_LIBRARIES)
set(VKPROFILER_LIB_DIR)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/vkProfiler)
set_target_properties(${VKPROFILER_LIBRARIES} PROPERTIES FOLDER Lumo_Libs/Static)
##set_target_properties(${VKPROFILER_LIBRARIES} PROPERTIES LINK_FLAGS "/ignore:4244")

message(STATUS "VKPROFILER_INCLUDE_DIR : ${VKPROFILER_INCLUDE_DIR}")
message(STATUS "VKPROFILER_LIBRARIES : ${VKPROFILER_LIBRARIES}")
message(STATUS "VKPROFILER_LIB_DIR : ${VKPROFILER_LIB_DIR}")


