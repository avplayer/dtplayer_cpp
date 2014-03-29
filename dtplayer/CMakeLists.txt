
include_directories(../dtdemux)
include_directories(../dtstream)
include_directories(../dtaudio)
include_directories(../dtvideo)

aux_source_directory( . LIBPLAYERSOURCES)

add_library(dtplayercore STATIC ${LIBPLAYERSOURCES})
