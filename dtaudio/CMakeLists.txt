

aux_source_directory( . LIBAUDIOSOURCES)

add_definitions(-DENABLE_AO_SDL)
add_definitions(-DENABLE_ADEC_FFMPEG)

if(FAAD_FOUND)
add_definitions(-DENABLE_ADEC_FAAD)
list(APPEND LIBAUDIOSOURCES  audio_decoder/dec_audio_faad.c)
endif()

add_library(dtaudio STATIC ${LIBAUDIOSOURCES} audio_out/ao_sdl.c audio_decoder/dec_audio_ffmpeg.c)

target_link_libraries(dtaudio  ${SDL2_LIBRARIES})

if(FAAD_FOUND)
target_link_libraries(dtaudio ${FAAD_LIBRARY})
endif()
