#ifndef FE_KIT_EXPORT_H
#define FE_KIT_EXPORT_H 1

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
#  if defined( FEKIT_LIBRARY_STATIC )
#    define FEKIT_EXPORT
#  elif defined( FEKIT_LIBRARY )
#    define FEKIT_EXPORT   __declspec(dllexport)
#  else
#    define FEKIT_EXPORT   __declspec(dllimport)
#endif
#else
#define FEKIT_EXPORT
#endif

#endif //FE_KIT_EXPORT_H