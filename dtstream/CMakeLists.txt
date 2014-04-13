
find_package( CURL )
find_package( Boost )

include_directories(../dtdemux)
include_directories(../dtstream)
include_directories(../dtaudio)
include_directories(../dtvideo)
include_directories(../dtport)

include_directories(${BT_INCLUDE_DIR})

aux_source_directory( . LIBSTREAMSOURCES)

add_definitions(-DENABLE_STREAM_FILE)

if (FFMPEG_FOUND)
list(APPEND LIBSTREAMSOURCES stream/stream_ffmpeg.cpp)
add_definitions(-DENABLE_STREAM_FFMPEG)
endif()

if (BT_FOUND)
list(APPEND LIBSTREAMSOURCES stream/stream_bt.cpp)
endif()

add_library(dtstream STATIC ${LIBSTREAMSOURCES} stream/stream_file.cpp)

if(FFMPEG_FOUND)
target_link_libraries(dtstream ${FFMPEG_LIBRARIES})
endif()

if(${BT_FOUND})
target_link_libraries( dtstream ${BT_LIBRARY} ${Boost_LIBRARIES})
endif()