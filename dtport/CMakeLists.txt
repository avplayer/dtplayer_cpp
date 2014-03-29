
include_directories(../dtdemux)
include_directories(../dtstream)
include_directories(../dtaudio)
include_directories(../dtvideo)
include_directories(../dtport)

aux_source_directory( . LIBPORTSOURCES)

add_library(dtport STATIC ${LIBPORTSOURCES})
