#ifndef HGALG_EXPORT_H
#define HGALG_EXPORT_H 1

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
#  if defined( FEALG_LIBRARY_STATIC )
#    define FEALG_EXPORT
#  elif defined( FEALG_LIBRARY )
#    define FEALG_EXPORT   __declspec(dllexport)
#  else
#    define FEALG_EXPORT   __declspec(dllimport)
#endif
#else
#define FEALG_EXPORT
#endif


#endif //HGALG_EXPORT_H