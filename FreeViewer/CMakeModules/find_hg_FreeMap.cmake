
SET(QGIS_DIR "" CACHE PATH "QGIS ")
MACRO( FIND_QGIS_INCLUDE THIS_QGIS_INCLUDE_DIR THIS_QGIS_INCLUDE_FILE )

FIND_PATH( ${THIS_QGIS_INCLUDE_DIR} ${THIS_QGIS_INCLUDE_FILE}
    PATHS
        ${OSGEARTH_DIR}
        $ENV{QGIS_SOURCE_DIR}
        $ENV{QGISDIR}
        $ENV{QGIS_DIR}
        /usr/local/
        /usr/
        /sw/ # Fink
        /opt/local/ # DarwinPorts
        /opt/csw/ # Blastwave
        /opt/
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;QGIS_ROOT]/
        ~/Library/Frameworks
        /Library/Frameworks
    PATH_SUFFIXES
        /include/
        /include/qgis
)

ENDMACRO( FIND_QGIS_INCLUDE THIS_QGIS_INCLUDE_DIR THIS_QGIS_INCLUDE_FILE  )

FIND_QGIS_INCLUDE( QGIS_INCLUDE_DIR       qgis.h )

###### libraries ######

MACRO( FIND_QGIS_LIBRARY MYLIBRARY MYLIBRARYNAME )

FIND_LIBRARY(${MYLIBRARY}
    NAMES
        ${MYLIBRARYNAME}
    PATHS
        ${OSGEARTH_DIR}
        $ENV{QGIS_BUILD_DIR}
        $ENV{QGIS_DIR}
        $ENV{QGISDIR}
        $ENV{QGIS_ROOT}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;QGIS_ROOT]/lib
        /usr/freeware
    PATH_SUFFIXES
        /lib/
        /lib64/
        /build/lib/
        /build/lib64/
        /Build/lib/
        /Build/lib64/
     )

ENDMACRO(FIND_QGIS_LIBRARY LIBRARY LIBRARYNAME)

FIND_QGIS_LIBRARY( QGIS_GUI_LIBRARY          qgis_gui)
FIND_QGIS_LIBRARY( QGIS_APP_LIBRARY          qgis_app)
FIND_QGIS_LIBRARY( QGIS_CORE_LIBRARY          qgis_core)
FIND_QGIS_LIBRARY(QGIS_NETWORKANALYSIS_LIBRARY qgis_networkanalysis)
FIND_QGIS_LIBRARY(QGIS_ANALYSIS_LIBRARY qgis_analysis)

FIND_QGIS_LIBRARY( QGIS_GUID_LIBRARY          qgis_guid)
FIND_QGIS_LIBRARY( QGIS_APPD_LIBRARY          qgis_appd)
FIND_QGIS_LIBRARY( QGIS_CORED_LIBRARY          qgis_cored)
FIND_QGIS_LIBRARY(QGIS_NETWORKANALYSISD_LIBRARY qgis_networkanalysisd)
FIND_QGIS_LIBRARY(QGIS_ANALYSISD_LIBRARY qgis_analysisd)

SET( QGIS_FOUND "NO" )
IF( QGIS_LIBRARY AND QGIS_INCLUDE_DIR )
    SET( QGIS_FOUND "YES" )
    SET( QGIS_INCLUDE_DIRS ${QGIS_INCLUDE_DIR})
    GET_FILENAME_COMPONENT( QGIS_LIBRARIES_DIR ${QGIS_LIBRARY} PATH )
ENDIF( QGIS_LIBRARY AND QGIS_INCLUDE_DIR )


