
include_directories(../dtdemux)
include_directories(../dtstream)
include_directories(../dtaudio)
include_directories(../dtvideo)
include_directories(../dtport)

aux_source_directory( . LIBSTREAMSOURCES)

add_definitions(-DENABLE_STREAM_FILE)
add_definitions(-DENABLE_STREAM_FFMPEG)

add_library(dtstream STATIC ${LIBSTREAMSOURCES} stream/stream_ffmpeg.c stream/stream_file.c)
