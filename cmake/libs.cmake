if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(X11 REQUIRED)
  if (NOT X11_Xi_FOUND)
    message(FATAL_ERROR "X11 Xi library is required")
  endif ()
endif ()

## aiekick
include(cmake/libs/gaia.cmake)
include(cmake/libs/lumobackend.cmake)
