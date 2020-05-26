#ifndef FE_EXT_NODE_EXPORT_H
#define FE_EXT_NODE_EXPORT_H 1

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__) || defined( __MWERKS__)
#  if defined( FEEXNODE_LIBRARY_STATIC )
#    define FEEXTNODE_EXPORT
#  elif defined( FEEXNODE_LIBRARY )
#    define FEEXTNODE_EXPORT   __declspec(dllexport)
#  else
#    define FEEXTNODE_EXPORT   __declspec(dllimport)
#endif
#else
#define FEEXTNODE_EXPORT
#endif


#endif //FE_EX_NODE_EXPORT_H