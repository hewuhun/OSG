
#ifndef FREEEARTH_VERSION
#define FREEEARTH_VERSION 1

#include <FeUtils/Export>

extern "C" {

#define FREEEARTH_MAJOR_VERSION    2
#define FREEEARTH_MINOR_VERSION    5
#define FREEEARTH_PATCH_VERSION    0
#define FREEEARTH_SOVERSION        0
#define FREEEARTH_RC_VERSION       0

/* Convenience macro that can be used to decide whether a feature is present or not i.e.
 * #if FREEEARTH_MIN_VERSION_REQUIRED(1,4,0)
 *    your code here
 * #endif
 */
#define FREEEARTH_MIN_VERSION_REQUIRED(MAJOR, MINOR, PATCH) ((FREEEARTH_MAJOR_VERSION>MAJOR) || (FREEEARTH_MAJOR_VERSION==MAJOR && (FREEEARTH_MINOR_VERSION>MINOR || (FREEEARTH_MINOR_VERSION==MINOR && FREEEARTH_PATCH_VERSION>=PATCH))))
#define FREEEARTH_VERSION_LESS_THAN(MAJOR, MINOR, PATCH) ((FREEEARTH_MAJOR_VERSION<MAJOR) || (FREEEARTH_MAJOR_VERSION==MAJOR && (FREEEARTH_MINOR_VERSION<MINOR || (FREEEARTH_MINOR_VERSION==MINOR && FREEEARTH_PATCH_VERSION<PATCH))))
#define FREEEARTH_VERSION_LESS_OR_EQUAL(MAJOR, MINOR, PATCH) ((FREEEARTH_MAJOR_VERSION<MAJOR) || (FREEEARTH_MAJOR_VERSION==MAJOR && (FREEEARTH_MINOR_VERSION<MINOR || (FREEEARTH_MINOR_VERSION==MINOR && FREEEARTH_PATCH_VERSION<=PATCH))))
#define FREEEARTH_VERSION_GREATER_THAN(MAJOR, MINOR, PATCH) ((FREEEARTH_MAJOR_VERSION>MAJOR) || (FREEEARTH_MAJOR_VERSION==MAJOR && (FREEEARTH_MINOR_VERSION>MINOR || (FREEEARTH_MINOR_VERSION==MINOR && FREEEARTH_PATCH_VERSION>PATCH))))
#define FREEEARTH_VERSION_GREATER_OR_EQUAL(MAJOR, MINOR, PATCH) ((FREEEARTH_MAJOR_VERSION>MAJOR) || (FREEEARTH_MAJOR_VERSION==MAJOR && (FREEEARTH_MINOR_VERSION>MINOR || (FREEEARTH_MINOR_VERSION==MINOR && FREEEARTH_PATCH_VERSION>=PATCH))))

/** embedded GIT SHA1 */
extern FEUTIL_EXPORT const char* FREEEARTHGitSHA1();

/**
  * FREEEARTHGetVersion() returns the library version number.
  * Numbering convention : FREEEARTH-1.0 will return 1.0 from FREEEARTHGetVersion.
  *
  * This C function can be also used to check for the existence of the FREEEARTH
  * library using autoconf and its m4 macro AC_CHECK_LIB.
  *
  * Here is the code to add to your configure.in:
 \verbatim
 #
 # Check for the FREEEARTH (OSG) library
 #
 AC_CHECK_LIB(osg, FREEEARTHGetVersion, ,
    [AC_MSG_ERROR(FREEEARTH library not found. See http://www.FREEEARTH.org)],)
 \endverbatim
*/
extern FREEEARTH_EXPORT const char* FREEEARTHGetVersion();

/** The osgGetSOVersion() method returns the FREEEARTH shared object version number. */
extern FREEEARTH_EXPORT const char* FREEEARTHGetSOVersion();

/** The osgGetLibraryName() method returns the library name in human-friendly form. */
extern FREEEARTH_EXPORT const char* FREEEARTHGetLibraryName();

#define FREEEARTH_VERSION_RELEASE FREEEARTH_VERSION_PATCH
#define FREEEARTH_VERSION_REVISION 0
}

#endif
