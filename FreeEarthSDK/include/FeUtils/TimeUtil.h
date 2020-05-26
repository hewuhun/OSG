/**************************************************************************************************
* @file TimeUtil.h
* @note 定义了获取系统时间的函数，可以获得时间的不同格式
* @author y00001
* @data 2014-7-16
**************************************************************************************************/

#ifndef _DEPEND_TIME_UTIL_H
#define _DEPEND_TIME_UTIL_H

#include <ctime>

#include <FeUtils/Export.h>

namespace FeUtil
{
    /**  
      * @brief 获得本地系统时间
      * @note 获得系统时间:年、月、日、小时、分钟、秒  
      * @param unYear [out]  年
      * @param unMonth [out]  月
      * @param unDay [out]  日
      * @param unHours [out]  小时
      * @param unMin [out]  分钟
      * @param unsecond [out]  秒
      * @return 返回值
    */
    extern FEUTIL_EXPORT void GetLocalYMDHMS(int& nYear, int& nMonth, int& nDay, int& nHours, int& nMinute, int& nSecond);

	/**  
      * @brief 转换本地UTC时间
      * @note 获得系统时间:年、月、日、小时、分钟、秒  
      * @param unYear [out]  年
      * @param unMonth [out]  月
      * @param unDay [out]  日
      * @param unHours [out]  小时
      * @param unMin [out]  分钟
      * @param unsecond [out]  秒
    */
    extern FEUTIL_EXPORT void Convert2YMDHMS(time_t t, int& nYear, int& nMonth, int& nDay, int& nHours, int& nMinute, int& nSecond);

    /**  
      * @brief 获得本地系统时间
      * @note 获得系统时间:年、月、日 
      * @param unYear [out]  年
      * @param unMonth [out]  月
      * @param unDay [out]  日
    */
    extern FEUTIL_EXPORT void GetLocalTimeYMD(int& nYear, int& nMonth, int& nDay);

     /**  
      * @brief 获得本地系统时间
      * @note 获得系统时间:小时、分钟、秒  
      * @param unHours [out]  小时
      * @param unMin [out]  分钟
      * @param unsecond [out]  秒
    */
    extern FEUTIL_EXPORT void GetLocalTimeHMS(int& nHours, int& nMinute, int& nSecond);


     /**  
      * @brief 获得UTC时间
      * @note 获得系统时间:年、月、日、小时、分钟、秒  
      * @param unYear [out]  年
      * @param unMonth [out]  月
      * @param unDay [out]  日
      * @param unHours [out]  小时
      * @param unMin [out]  分钟
      * @param unsecond [out]  秒
    */
    extern FEUTIL_EXPORT void GetUTCTimeYMDHMS(int& nYear, int& nMonth, int& nDay, int& nHours, int& nMinute, int& nSecond);


     /**  
      * @brief 获得UTC时间
      * @note 获得系统时间:年、月、日 
      * @param unYear [out]  年
      * @param unMonth [out]  月
      * @param unDay [out]  日
    */
    extern FEUTIL_EXPORT void GetUTCTimeYMD(int& nYear, int& nMonth, int& nDay);

     /**  
      * @brief 获得UTC时间
      * @note 获得系统时间:小时、分钟、秒  
      * @param unHours [out]  小时
      * @param unMin [out]  分钟
      * @param unsecond [out]  秒
    */
    extern FEUTIL_EXPORT void GetUTCTimeHMS(int& nHours, int& nMinute, int& nSecond);

    /**  
      * @brief 获得本地时间
      * @note 获得本地时间数据块
      * @return 时间数据块
    */
    extern FEUTIL_EXPORT tm GetLocalTime();
    
    /**  
      * @brief 获得UTC时间
      * @note 获得本地时间数据块
      * @return 时间数据块
    */
    extern FEUTIL_EXPORT tm GetUTCTime();

     /**  
      * @brief 当前时间转UTC时间
      * @note 当前时间转UTC时间
      * @return 
    */
    extern FEUTIL_EXPORT tm Local2UTC(tm localTM);
    
     /**  
      * @brief UTC时间转当前时间
      * @note UTC时间转当前时间
      * @return 
    */
    extern FEUTIL_EXPORT tm UTC2Local(tm UTCTM);

     /**  
      * @brief DateTime转UTC时间
      * @note DateTime转UTC时间
      * @return 
    */
    extern FEUTIL_EXPORT tm DataTime2UTC(int nYear, int nMonth, int nDay, int nHours);

     /**  
      * @brief DateTime转UTC时间
      * @note DateTime转UTC时间
      * @return 
    */
    extern FEUTIL_EXPORT void UTC2DataTime(tm& UTCMT, int& nYear, int& nMonth, int& nDay, int& nHours);

	/**  
      * @brief 判断是否上午
      * @note 是否上午
      * @return 上午返回true，下午返回false
    */
    extern FEUTIL_EXPORT bool IsAm(int nHours);
}

#endif //_DEPEND_TIME_UTIL_H
