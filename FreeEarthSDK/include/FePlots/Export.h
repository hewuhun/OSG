#ifndef FE_PLOTS_EXPORT_H
#define FE_PLOTS_EXPORT_H 1

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
#  if defined( FEPLOTS_LIBRARY_STATIC )
#    define FEPLOTS_EXPORT
#  elif defined( FEPLOTS_LIBRARY )
#    define FEPLOTS_EXPORT   __declspec(dllexport)
#  else
#    define FEPLOTS_EXPORT   __declspec(dllimport)
#endif
#else
#define FEPLOTS_EXPORT
#endif


#endif