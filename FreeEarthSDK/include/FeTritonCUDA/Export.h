#ifndef FE_TRITONCUDA_EXPORT_H
#define FE_TRITONCUDA_EXPORT_H 1

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
#  if defined( CUDA_LIBRARY_STATIC )
#    define FETRITONCUDA_EXPORT
#  elif defined( CUDA_LIBRARY )
#    define FETRITONCUDA_EXPORT   __declspec(dllexport)
#  else
#    define FETRITONCUDA_EXPORT   __declspec(dllimport)
#endif
#else
#define FETRITONCUDA_EXPORT
#endif

#endif 