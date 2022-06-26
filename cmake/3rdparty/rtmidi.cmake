set(RTMIDI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/rtmidi)
set(RTMIDI_LIBRARIES ${RTMIDI_LIBRARIES} rtmidi)

add_subdirectory(${RTMIDI_INCLUDE_DIR})
set_target_properties(${RTMIDI_LIBRARIES} PROPERTIES FOLDER 3rdparty)


