#----------------------------------------------------------------
# Generated CMake target import file for configuration "MinSizeRel".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Tracy::TracyClient" for configuration "MinSizeRel"
set_property(TARGET Tracy::TracyClient APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(Tracy::TracyClient PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/lib/TracyClient.lib"
  )

list(APPEND _cmake_import_check_targets Tracy::TracyClient )
list(APPEND _cmake_import_check_files_for_Tracy::TracyClient "${_IMPORT_PREFIX}/lib/TracyClient.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
