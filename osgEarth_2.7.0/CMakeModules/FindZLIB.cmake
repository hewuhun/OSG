# Locate ZLIB.
# This module defines
# ZLIB_LIBRARY
# ZLIB_LIBRARY_DEBUG
# ZLIB_FOUND, if false, do not try to link to zlib
# ZLIB_INCLUDE_DIR, where to find the headers

FIND_PATH(ZLIB_INCLUDE_DIR zlib.h
  $ENV{ZLIB_DIR}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(ZLIB_INCLUDE_DIR zlib.h
  PATHS
  ~/Library/Frameworks/zlib/Headers
  /Library/Frameworks/zlib/Headers
  /usr/local/include/zlib
  /usr/local/include/ZLIB
  /usr/local/include
  /usr/include/zlib
  /usr/include/ZLIB
  /usr/include
  /sw/include/zlib 
  /sw/include/ZLIB 
  /sw/include # Fink
  /opt/local/include/zlib
  /opt/local/include/ZLIB
  /opt/local/include # DarwinPorts
  /opt/csw/include/zlib
  /opt/csw/include/ZLIB
  /opt/csw/include # Blastwave
  /opt/include/zlib
  /opt/include/ZLIB
  /opt/include
  e:/devel/zlib-3.1.1/source/headers
)

FIND_LIBRARY(ZLIB_LIBRARY
  NAMES zlib
  PATHS
    $ENV{ZLIB_DIR}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

FIND_LIBRARY(ZLIB_LIBRARY
  NAMES zlib
  PATHS
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    /usr/freeware    
  PATH_SUFFIXES lib64 lib
)


FIND_LIBRARY(ZLIB_LIBRARY_DEBUG
  NAMES zlibd
  PATHS
    $ENV{ZLIB_DIR}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

FIND_LIBRARY(ZLIB_LIBRARY_DEBUG
  NAMES zlibd
  PATHS
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    /usr/freeware    
  PATH_SUFFIXES lib64 lib
)

SET(ZLIB_FOUND "NO")
IF(ZLIB_LIBRARY AND ZLIB_INCLUDE_DIR)
  SET(ZLIB_FOUND "YES")
ENDIF(ZLIB_LIBRARY AND ZLIB_INCLUDE_DIR)

