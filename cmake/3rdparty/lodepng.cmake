set(LODEPNG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/lodepng)

file(GLOB LODEPNG_SOURCES 
	${LODEPNG_INCLUDE_DIR}/lodepng.cpp
	${LODEPNG_INCLUDE_DIR}/lodepng.h)
source_group(src FILES ${LODEPNG_SOURCES})		
   
add_library(lodepng STATIC ${LODEPNG_SOURCES})

include_directories(${LODEPNG_INCLUDE_DIR})
    
set_target_properties(lodepng PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(lodepng PROPERTIES FOLDER 3rdparty)

set(LODEPNG_LIBRARIES lodepng)