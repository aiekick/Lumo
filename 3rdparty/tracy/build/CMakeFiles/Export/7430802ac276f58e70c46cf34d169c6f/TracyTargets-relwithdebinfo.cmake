#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Tracy::TracyClient" for configuration "RelWithDebInfo"
set_property(TARGET Tracy::TracyClient APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(Tracy::TracyClient PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/TracyClient.lib"
  )

list(APPEND _cmake_import_check_targets Tracy::TracyClient )
list(APPEND _cmake_import_check_files_for_Tracy::TracyClient "${_IMPORT_PREFIX}/lib/TracyClient.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
