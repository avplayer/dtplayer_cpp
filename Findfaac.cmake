# Locate libfaac library      
# This module defines
# FAAC_LIBRARY, the name of the library to link against
# FAAC_FOUND, if false, do not try to link
# FAAC_INCLUDE_DIR, where to find header
#

set( FAAC_FOUND "NO" )

find_path( FAAC_INCLUDE_DIR faac.h
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

find_library( FAAC_LIBRARY
  NAMES faac
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

if(FAAC_LIBRARY)
set( FAAC_FOUND "YES" )
endif(FAAC_LIBRARY)

