/**************************************************************************************************
* @file HgSharpOptions.h
* @note Gis的配置项
* @author c00005
* @data 2013-12-20
**************************************************************************************************/
#ifndef HG_SHARP_OPTIONS_H
#define HG_SHARP_OPTIONS_H

#include <osgEarth/Common>
#include <osgEarth/TileSource>
#include <osgEarth/URI>

#include <PluginDependent/Export.h>

namespace FePlugin
{
    using namespace osgEarth;

    /**
    * @class HgGISOptions
    * @brief Gis的配置项
    * @note Gis的配置项，继承自TileSourceOptions，其中存储了瓦片的格式、URL以及缓存路径
    * @author c00005
    */
    class PLUGIN_EXPORT CHgSharpOptions : public TileSourceOptions
    {
    public:
        /**  
        * @brief 构造函数
        * @note 带参数的构造函数  
        * @param opt [in] 传入全局的Options，用于构造本类对象
        * @return 无
        */
        CHgSharpOptions( const TileSourceOptions& opt =TileSourceOptions() );

        /**  
        * @brief 构造函数
        * @note 拷贝构造函数  
        * @param rhs [in] CHgSharpOptions类的对象引用，用来初始化类中变量的值
        * @return 无
        */
        CHgSharpOptions( const CHgSharpOptions& rhs );

        /**  
        * @brief 析构函数
        * @note 虚析构函数
        * @return 无
        */
        virtual ~CHgSharpOptions();

    public:
        /**  
        * @note 返回文件路径
        */
        optional<URI>& GetUrl() { return m_urlPath; }

        /**  
        * @note 返回文件路径，并且不可修改
        */
        const optional<URI>& GetUrl() const { return m_urlPath; }

        /**  
        * @note 返回标记象征
        */
        optional<std::string>& GetToken() { return m_strToken; }

        /**  
        * @note 返回标记象征，并且不可修改
        */
        const optional<std::string>& GetToken() const { return m_strToken; }

        /**  
        * @note 返回格式
        */
        optional<std::string>& GetFormat() { return m_strFormat; }

        /**  
        * @note 返回格式，并且不可修改
        */
        const optional<std::string>& GetFormat() const { return m_strFormat; }

        /**  
        * @note 返回缓存路径
        */
        optional<std::string>& GetCacheUrl() { return m_strCachePath; }

        /**  
        * @note 返回缓存路径，并且不可修改
        */
        const optional<std::string>& GetCacheUrl() const { return m_strCachePath; }

    public:
        /**  
        * @brief 获取conf中的信息
        * @note 获取conf中的信息
        * @return Config 返回的是整合在一起的config信息
        */
        Config GetConfig() const;

        /**  
        * @brief 改变conf中的信息
        * @note 改变conf中的信息，并改变本类中存储的信息
        * @param conf [in] 输入要被改变的conf对象
        * @return 无
        */
        void MergeConfig( const Config& conf );

        /**  
        * @brief 从conf获取配置信息
        * @note 从传入的conf中获取到配置信息
        * @param conf [in] 用来获取其中的配置信息
        * @return 无
        */
        void FromConfig( const Config& conf );

    private:
        ///存储文件路径的url
        optional<URI>         m_urlPath;
        ///存储安全标记
        optional<std::string> m_strToken;
        ///存储格式
        optional<std::string> m_strFormat;
        ///缓存路径
        optional<std::string> m_strCachePath;
    };

}

#endif 


