#ifndef FE_LAYERS_EXPORT_H
#define FE_LAYERS_EXPORT_H

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( SHELL_LIBRARY_STATIC )
    #    define FELAYERS_EXPORT
    #  elif defined( SHELL_LIBRARY )
    #    define FELAYERS_EXPORT   __declspec(dllexport)
    #  else
    #    define FELAYERS_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define FELAYERS_EXPORT
#endif  

#endif //FE_LAYERS_EXPORT_H
