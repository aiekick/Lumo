set(VOLK_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/volk)
file(GLOB VOLK_SOURCES ${CMAKE_SOURCE_DIR}/3rdparty/volk/volk.c)
file(GLOB VOLK_HEADERS ${VOLK_INCLUDE_DIR}/volk/volk.h)

add_library(volk STATIC ${VOLK_HEADERS} ${VOLK_SOURCES})

include_directories(${VOLK_INCLUDE_DIR})

set_target_properties(volk PROPERTIES LINKER_LANGUAGE C)
set_target_properties(volk PROPERTIES FOLDER 3rdparty)

set(VOLK_LIBRARIES volk)

if(NOT WIN32)
    set(VOLK_LIBRARIES ${VOLK_LIBRARIES} dl)
endif()
