
SET(FREESERVER_DIR "" CACHE PATH "Location of FreeServer SDK")
MACRO( FIND_FREESERVER_INCLUDE THIS_FREESERVER_INCLUDE_DIR THIS_FREESERVER_INCLUDE_FILE )

FIND_PATH( ${THIS_FREESERVER_INCLUDE_DIR} ${THIS_FREESERVER_INCLUDE_FILE}
    PATHS
        ${FREESERVER_DIR}
        $ENV{FREESERVER_SOURCE_DIR}
        $ENV{FREESERVERDIR}
        $ENV{FREESERVER_DIR}
        /usr/local/
        /usr/
        /sw/ # Fink
        /opt/local/ # DarwinPorts
        /opt/csw/ # Blastwave
        /opt/
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;FREESERVER_ROOT]/
        ~/Library/Frameworks
        /Library/Frameworks
    PATH_SUFFIXES
        /include/
)

ENDMACRO( FIND_FREESERVER_INCLUDE THIS_FREESERVER_INCLUDE_DIR THIS_FREESERVER_INCLUDE_FILE )

FIND_FREESERVER_INCLUDE( FREESERVER_INCLUDE_DIR       FsDataDef/Export.h )

###### libraries ######

MACRO( FIND_FREESERVER_LIBRARY MYLIBRARY MYLIBRARYNAME )

FIND_LIBRARY(${MYLIBRARY}
    NAMES
        ${MYLIBRARYNAME}
    PATHS
        ${FREESERVER_DIR}
        $ENV{FREESERVER_BUILD_DIR}
        $ENV{FREESERVER_DIR}
        $ENV{FREESERVERDIR}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;FREESERVER_ROOT]/lib
        /usr/freeware
    PATH_SUFFIXES
        /lib/
        /lib64/
        /build/lib/
        /build/lib64/
        /Build/lib/
        /Build/lib64/
     )

ENDMACRO(FIND_FREESERVER_LIBRARY LIBRARY LIBRARYNAME)

FIND_FREESERVER_LIBRARY( FsDataDef_DEBUG_LIBRARIES           FsDataDefd)
FIND_FREESERVER_LIBRARY( FsUDPService_DEBUG_LIBRARIES   	  FsUDPServiced)
FIND_FREESERVER_LIBRARY( FsDataDef_RELEASE_LIBRARIES         FsDataDef)
FIND_FREESERVER_LIBRARY( FsUDPService_RELEASE_LIBRARIES   	  FsUDPService)


SET( FREESERVER_FOUND "NO" )
IF( FREESERVER_LIBRARY AND FREESERVER_INCLUDE_DIR )
    SET( FREESERVER_FOUND "YES" )
    SET( FREESERVER_INCLUDE_DIRS ${FREESERVER_INCLUDE_DIR})
    GET_FILENAME_COMPONENT( FREESERVER_LIBRARIES_DIR ${FREESERVER_LIBRARY} PATH )
ENDIF( FREESERVER_LIBRARY AND FREESERVER_INCLUDE_DIR )


