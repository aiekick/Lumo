file(GLOB LFWATCH_FILES 
	${CMAKE_SOURCE_DIR}/3rdparty/lfwatch/src/*.h 
	${CMAKE_SOURCE_DIR}/3rdparty/lfwatch/src/*.cpp)
add_library(lfwatch STATIC ${LFWATCH_FILES})
set_target_properties(lfwatch PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(lfwatch PROPERTIES FOLDER 3rdparty)
set(LFWATCH_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/lfwatch/include)
set(LFWATCH_LIBRARIES lfwatch)
