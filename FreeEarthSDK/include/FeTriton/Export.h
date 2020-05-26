#ifndef FE_TRITON_EXPORT_H
#define FE_TRITON_EXPORT_H 1

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
#  if defined( TRITON_LIBRARY_STATIC )
#    define FETRITON_EXPORT
#  elif defined( TRITON_LIBRARY )
#    define FETRITON_EXPORT   __declspec(dllexport)
#  else
#    define FETRITON_EXPORT   __declspec(dllimport)
#endif
#else
#define FETRITON_EXPORT
#endif

#endif 