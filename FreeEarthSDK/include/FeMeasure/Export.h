#ifndef FE_MEASURE_EXPORT_H
#define FE_MEASURE_EXPORT_H 1

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
#  if defined( FEMEASURE_LIBRARY_STATIC )
#    define FEMEASURE_EXPORT
#  elif defined( FEMEASURE_LIBRARY )
#    define FEMEASURE_EXPORT   __declspec(dllexport)
#  else
#    define FEMEASURE_EXPORT   __declspec(dllimport)
#endif
#else
#define FEMEASURE_EXPORT
#endif

#endif //FE_MEASURE_EXPORT_H