
include_directories(../dtdemux)
include_directories(../dtstream)
include_directories(../dtaudio)
include_directories(../dtvideo)
include_directories(../dtport)

aux_source_directory( . LIBSTREAMSOURCES)

add_definitions(-DENABLE_STREAM_FILE)

if (FFMPEG_FOUND)
list(APPEND LIBSTREAMSOURCES stream/stream_ffmpeg.c)
add_definitions(-DENABLE_STREAM_FFMPEG)
endif()

add_library(dtstream STATIC ${LIBSTREAMSOURCES} stream/stream_file.c)

if(FFMPEG_FOUND)
target_link_libraries(dtstream ${FFMPEG_LIBRARIES})
endif()