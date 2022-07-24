include(FindPythonInterp)
execute_process(
	COMMAND ${PYTHON_EXECUTABLE} update_glslang_sources.py
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/glslang)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_OPT ON CACHE BOOL "" FORCE)
set(ENABLE_PCH ON CACHE BOOL "" FORCE)
set(ENABLE_CTEST OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES ON CACHE BOOL "" FORCE)
set(ENABLE_SPVREMAPPER ON CACHE BOOL "" FORCE)
set(BUILD_EXTERNAL ON CACHE BOOL "" FORCE)
set(ENABLE_HLSL ON CACHE BOOL "" FORCE)
set(USE_CCACHE ON CACHE BOOL "" FORCE)	
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/glslang)

set_target_properties(glslang PROPERTIES FOLDER 3rdparty/glslang)
set_target_properties(GenericCodeGen PROPERTIES FOLDER 3rdparty/glslang)
set_target_properties(MachineIndependent PROPERTIES FOLDER 3rdparty/glslang)
set_target_properties(OGLCompiler PROPERTIES FOLDER 3rdparty/glslang)
set_target_properties(OSDependent PROPERTIES FOLDER 3rdparty/glslang)
set_target_properties(SPIRV PROPERTIES FOLDER 3rdparty/glslang)
set_target_properties(glslang-default-resource-limits PROPERTIES FOLDER 3rdparty/glslang)
set_target_properties(HLSL PROPERTIES FOLDER 3rdparty/glslang)
set_target_properties(glslangValidator PROPERTIES FOLDER 3rdparty/glslang/apps)

set(GLSLANG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/glslang/include)
set(GLSLANG_LIBRARIES glslang SPIRV)