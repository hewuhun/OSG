#ifndef HGUTIL_EXPORT_H
#define HGUTIL_EXPORT_H 1

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
#  if defined( FEUTILS_LIBRARY_STATIC )
#    define FEUTIL_EXPORT
#  elif defined( FEUTILS_LIBRARY )
#    define FEUTIL_EXPORT   __declspec(dllexport)
#  else
#    define FEUTIL_EXPORT   __declspec(dllimport)
#endif
#else
#define FEUTIL_EXPORT
#endif


#endif //FEUTIL_EXPORT_H