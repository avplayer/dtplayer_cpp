
include_directories(../dtdemux)
include_directories(../dtstream)
include_directories(../dtaudio)
include_directories(../dtvideo)
include_directories(../dtport)

aux_source_directory( . LIBDEMUXSOURCES)

if(FFMPEG_FOUND)
add_definitions(-DENABLE_DEMUXER_FFMPEG)
list(APPEND LIBDEMUXSOURCES  demuxer/demuxer_ffmpeg.c)
endif()

add_definitions(-DENABLE_DEMUXER_AAC)
list(APPEND LIBDEMUXSOURCES demuxer/demuxer_aac.c)

add_library(dtdemux STATIC ${LIBDEMUXSOURCES} )

if(FFMPEG_FOUND)
target_link_libraries(dtdemux ${FFMPEG_LIBRARIES})
endif()

if(FAAC_FOUND)
target_link_libraries(dtdemux ${FAAC_LIBRARY})
endif()
