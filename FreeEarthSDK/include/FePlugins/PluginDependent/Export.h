/**************************************************************************************************
* @file Export.h
* @note 导出函数
* @author l00008
* @data 2013-12-30
**************************************************************************************************/

#ifndef FE_PLUGIN_EXPORT_H
#define FE_PLUGIN_EXPORT_H

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #  if defined( FEPLUGIN_LIBRARY_STATIC )
    #    define PLUGIN_EXPORT
    #  elif defined( FEPLUGIN_LIBRARY )
    #    define PLUGIN_EXPORT   __declspec(dllexport)
    #  else
    #    define PLUGIN_EXPORT   __declspec(dllimport)
    #  endif
#else
    #  define PLUGIN_EXPORT
#endif  

#endif //FE_PLUGIN_EXPORT_H
