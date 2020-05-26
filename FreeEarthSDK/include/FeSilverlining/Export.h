#ifndef FESILVERLINING_EXPORT_H
#define FESILVERLINING_EXPORT_H

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( FESILVERLINING_LIBRARY_STATIC )
    #    define FESILVERLINING_EXPORT
    #  elif defined( FESILVERLINING_LIBRARY )
    #    define FESILVERLINING_EXPORT   __declspec(dllexport)
    #  else
    #    define FESILVERLINING_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define FESILVERLINING_EXPORT
#endif  

#endif //FESILVERLINING_EXPORT_H