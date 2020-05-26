#ifndef FE_MANAGER_EXPORT_H
#define FE_MANAGER_EXPORT_H 1

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
#  if defined( FEMANAGER_LIBRARY_STATIC )
#    define FEMANAGER_EXPORT
#  elif defined( FEMANAGER_LIBRARY )
#    define FEMANAGER_EXPORT   __declspec(dllexport)
#  else
#    define FEMANAGER_EXPORT   __declspec(dllimport)
#endif
#else
#define FEMANAGER_EXPORT
#endif


#endif