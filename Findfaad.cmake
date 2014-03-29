# Locate libfaad library    
# This module defines
# FAAD_LIBRARY, the name of the library to link against
# FAAD_FOUND, if false, do not try to link
# FAAD_INCLUDE_DIR, where to find header
#

set( FAAD_FOUND "NO" )

find_path( FAAD_INCLUDE_DIR faad.h
  HINTS
  PATH_SUFFIXES include 
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include 
  /opt/include
  /mingw
)

find_library( FAAD_LIBRARY
  NAMES faad
  HINTS
  PATH_SUFFIXES lib64 lib
  PATHS
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
  /mingw
)

if(FAAD_LIBRARY)
set( FAAD_FOUND "YES" )
endif(FAAD_LIBRARY)

