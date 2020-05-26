#ifndef FREE_SAMPLE_FRAME_EXPORT_H
#define FREE_SAMPLE_FRAME_EXPORT_H

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( EARTH_LIBRARY_STATIC )
    #    define FREE_SAMPLE_FRAME_EXPORT
    #  elif defined( EARTH_LIBRARY )
    #    define FREE_SAMPLE_FRAME_EXPORT   __declspec(dllexport)
    #  else
    #    define FREE_SAMPLE_FRAME_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define FREE_SAMPLE_FRAME_EXPORT
#endif  

#endif //FREE_EARTH_EXPORT_H
