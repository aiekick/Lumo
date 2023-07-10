set(GAIA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/Gaia/include)
set(GAIA_LIBRARIES Gaia)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/Gaia)

set_target_properties(Gaia PROPERTIES FOLDER Lumo_Libs/Shared)
set_target_properties(Gaia PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(Gaia PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(Gaia PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
