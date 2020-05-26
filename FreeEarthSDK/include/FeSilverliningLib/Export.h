#ifndef FESILVERLININGLIB_EXPORT_H
#define FESILVERLININGLIB_EXPORT_H

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( FESILVERLININGLIB_LIBRARY_STATIC )
    #    define FESILVERLININGLIB_EXPORT
    #  elif defined( FESILVERLININGLIB_LIBRARY )
    #    define FESILVERLININGLIB_EXPORT   __declspec(dllexport)
    #  else
    #    define FESILVERLININGLIB_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define FESILVERLININGLIB_EXPORT
#endif  

#endif //FESILVERLININGLIB_EXPORT_H