

aux_source_directory( . LIBAUDIOSOURCES)

add_definitions(-DENABLE_AO_SDL2)

if(FFMPEG_FOUND)
add_definitions(-DENABLE_ADEC_FFMPEG)
list(APPEND LIBAUDIOSOURCES audio_decoder/dec_audio_ffmpeg.c)
endif()


if(FAAD_FOUND)
add_definitions(-DENABLE_ADEC_FAAD)
list(APPEND LIBAUDIOSOURCES  audio_decoder/dec_audio_faad.c)
endif()

add_library(dtaudio STATIC ${LIBAUDIOSOURCES} audio_out/ao_sdl2.c)

target_link_libraries(dtaudio  ${SDL2_LIBRARIES})

if(FAAD_FOUND)
target_link_libraries(dtaudio ${FAAD_LIBRARY})
endif()

if(FFMPEG_FOUND)
target_link_libraries(dtaudio ${FFMPEG_LIBRARIES})
endif()

