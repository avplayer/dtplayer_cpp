# Locate libtorrent library    
# This module defines
# BT_LIBRARY, the name of the library to link against
# BT_FOUND, if false, do not try to link
# BT_INCLUDE_DIR, where to find header
#

set( BT_FOUND "NO" )

find_path( BT_INCLUDE_DIR /libtorrent/torrent.hpp
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

find_library( BT_LIBRARY
  NAMES torrent-rasterbar
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

if(BT_LIBRARY)
message(STATUS "Found LibTorrent: ${BT_LIBRARY}, ${BT_INCLUDE_DIR}")
add_definitions(-DBOOST_ASIO_DYN_LINK)
set( BT_FOUND "YES" )
endif(BT_LIBRARY)

