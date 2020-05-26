#ifndef FE_EFFECTS_EXPORT_H
#define FE_EFFECTS_EXPORT_H 1

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
#  if defined( FEEFFECTS_LIBRARY_STATIC )
#    define FEEFFECTS_EXPORT
#  elif defined( FEEFFECTS_LIBRARY )
#    define FEEFFECTS_EXPORT   __declspec(dllexport)
#  else
#    define FEEFFECTS_EXPORT   __declspec(dllimport)
#endif
#else
#define FEEFFECTS_EXPORT
#endif

#endif //FE_EFFECTS_EXPORT_H