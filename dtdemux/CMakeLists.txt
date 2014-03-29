
include_directories(../dtdemux)
include_directories(../dtstream)
include_directories(../dtaudio)
include_directories(../dtvideo)
include_directories(../dtport)

add_definitions(-DENABLE_DEMUXER_FFMPEG -DENABLE_DEMUXER_AAC)

aux_source_directory( . LIBDEMUXSOURCES)

add_library(dtdemux STATIC ${LIBDEMUXSOURCES} demuxer/demuxer_aac.c demuxer/demuxer_ffmpeg.c)

target_link_libraries(dtdemux ${FFMPEG_LIBRARIES})

