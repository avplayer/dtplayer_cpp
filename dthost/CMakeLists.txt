
include_directories(../dtdemux)
include_directories(../dtstream)
include_directories(../dtaudio)
include_directories(../dtvideo)
include_directories(../dtport)

aux_source_directory( . LIBHOSTSOURCES)

add_library(dthost STATIC ${LIBHOSTSOURCES})
