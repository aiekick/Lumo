set(MINIAUDIO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/miniaudio/extras/miniaudio_split)
file(GLOB MINIAUDIO_SOURCES 
	${MINIAUDIO_INCLUDE_DIR}/*.c
	${MINIAUDIO_INCLUDE_DIR}/*.h
)

add_library(miniaudio STATIC ${MINIAUDIO_SOURCES})

include_directories(${MINIAUDIO_INCLUDE_DIR})
    
set_target_properties(miniaudio PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(miniaudio PROPERTIES FOLDER 3rdparty/audio)

set(MINIAUDIO_LIBRARIES miniaudio)
