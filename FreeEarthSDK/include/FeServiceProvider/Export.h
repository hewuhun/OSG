#ifndef FE_SERVICEPROVIDER_EXPORT_H
#define FE_SERVICEPROVIDER_EXPORT_H 1

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
#  if defined( FESERVICEPROVIDER_LIBRARY_STATIC )
#    define FESERVICEPROVIDER_EXPORT
#  elif defined( FESERVICEPROVIDER_LIBRARY )
#    define FESERVICEPROVIDER_EXPORT   __declspec(dllexport)
#  else
#    define FESERVICEPROVIDER_EXPORT   __declspec(dllimport)
#endif
#else
#define FESERVICEPROVIDER_EXPORT
#endif

#endif //FE_SERVICEPROVIDER_EXPORT_H