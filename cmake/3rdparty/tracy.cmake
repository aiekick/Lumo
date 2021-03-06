file(GLOB TRACY_SOURCES ${CMAKE_SOURCE_DIR}/3rdparty/tracy/TracyClient.cpp)

add_library(tracy STATIC ${TRACY_SOURCES})

set_target_properties(tracy PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(tracy PROPERTIES FOLDER 3rdparty)

set(TRACY_LIBRARIES tracy)

if(NOT WIN32)
    set(TRACY_LIBRARIES ${TRACY_LIBRARIES} dl pthread)
endif()
