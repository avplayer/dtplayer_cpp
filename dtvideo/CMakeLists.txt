
aux_source_directory( . LIBVIDEOSOURCES)

add_definitions(-DENABLE_VO_SDL2)
add_definitions(-DENABLE_VDEC_FFMPEG)

add_library(dtvideo STATIC ${LIBVIDEOSOURCES} video_out/vo_sdl2.c video_decoder/dec_video_ffmpeg.c)
target_link_libraries(dtvideo  ${SDL2_LIBRARIES})
