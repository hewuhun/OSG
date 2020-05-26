/**************************************************************************************************
* @file StrUtil.h
* @note 定义了字符串的一些方法
* @author y00001
* @data 2013-12-31
**************************************************************************************************/
#ifndef STRUTIL_H
#define STRUTIL_H

#include <vector>
#include <string>

#include <FeUtils/Export.h>

using namespace std;   

namespace FeUtil
{
    /** 
      * @brief 对字符串进行替换，注意：替换后若有旧值，仍然会替换为新值，比如string("12212"),"12","21"替换结果为：22211
      * @param str源串
      * @param old_value旧值
      * @param new_value新值
      * @return str结果
    */
    extern FEUTIL_EXPORT string StringReplaceAll(string str, const string& old_value, const string& new_value);   

    /**  
      * @brief 对字符串进行替换，仅替换一次，比如string("12212"),"12","21"替换结果为：21221
      * @param str源串
      * @param old_value旧值
      * @param new_value新值
      * @return str结果
    */
    extern FEUTIL_EXPORT string StringReplace(string str, const string& old_value, const string& new_value); 

    /**  
      * @brief 格式化路径字符串，将所有路径规范化为D:/a/b/，也即末尾带/且都是/不含\\
    */
    extern FEUTIL_EXPORT string StringDirectoryFmt(string str); 

    /**  
      * @brief 将string按某个字符分隔，比如"a b"，delim为" "，则ret为"a","b"
      * @param 参数 s源串
      * @param 参数 delim分隔符
      * @param 出参 子string串
    */
    extern FEUTIL_EXPORT void StringSplit(const std::string& s, const std::string& delim, std::vector< std::string >& ret);

    /**  
      * @brief 将字符串全变大写
    */
    extern FEUTIL_EXPORT std::string StringUpper(std::string s);

    /**  
      * @brief 将字符串全变小写
    */
    extern FEUTIL_EXPORT std::string StringLower(std::string s);

    extern FEUTIL_EXPORT bool StringToBool(const std::string& str);

    extern FEUTIL_EXPORT std::string BoolToString(bool bState);

    extern FEUTIL_EXPORT double StringToDouble(const std::string& str);

    extern FEUTIL_EXPORT std::string DoubleToString(double dData);

    extern FEUTIL_EXPORT int StringToInt(const std::string& str);

    extern FEUTIL_EXPORT std::string IntToString(int bState);

    /**  
      * @brief 将字符串转为UTF8
    */
    extern FEUTIL_EXPORT std::string ToUTF(const std::string& str);

	/**  
      * @brief 将字符串转为UTF8
    */
    extern FEUTIL_EXPORT std::string UTFToCurCode(const std::string& str);
	
}


#endif
