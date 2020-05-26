#ifndef FREE_EARTH_EXPORT_H
#define FREE_EARTH_EXPORT_H



#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( EARTH_LIBRARY_STATIC )
    #    define FEEARTH_EXPORT
    #  elif defined( EARTH_LIBRARY )
    #    define FEEARTH_EXPORT   __declspec(dllexport)
    #  else
    #    define FEEARTH_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define FEEARTH_EXPORT
#endif  

#endif //FREE_EARTH_EXPORT_H
