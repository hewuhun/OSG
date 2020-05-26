#ifndef FEOCEAN_EXPORT_H
#define FEOCEAN_EXPORT_H



#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( OCEAN_LIBRARY_STATIC )
    #    define FEOCEAN_EXPORT
    #  elif defined( OCEAN_LIBRARY )
    #    define FEOCEAN_EXPORT   __declspec(dllexport)
    #  else
    #    define FEOCEAN_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define FEOCEAN_EXPORT
#endif  

#endif 