
aux_source_directory( . LIBVIDEOSOURCES)

add_definitions(-DENABLE_VO_SDL2)

if(FFMPEG_FOUND)
add_definitions(-DENABLE_VDEC_FFMPEG)
list(APPEND LIBVIDEOSOURCES video_decoder/dec_video_ffmpeg.cpp)
endif()

add_library(dtvideo STATIC ${LIBVIDEOSOURCES})
target_link_libraries(dtvideo  ${SDL2_LIBRARIES})

if(FFMPEG_FOUND)
target_link_libraries(dtvideo ${FFMPEG_LIBRARIES})
endif()
