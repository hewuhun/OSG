#ifndef FE_SHELL_EXPORT_H
#define FE_SHELL_EXPORT_H

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( SHELL_LIBRARY_STATIC )
    #    define FESHELL_EXPORT
    #  elif defined( SHELL_LIBRARY )
    #    define FESHELL_EXPORT   __declspec(dllexport)
    #  else
    #    define FESHELL_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define FESHELL_EXPORT
#endif  

#endif //FE_SHELL_EXPORT_H
