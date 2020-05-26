/**************************************************************************************************
* @file FlatGisOptions.h
* @note 平顶四叉树的配置选项
* @author c00005
* @data 2013-12-24
**************************************************************************************************/
#ifndef FLATISOPTIONS_H
#define FLATISOPTIONS_H

#include <osgEarth/Common>
#include <osgEarth/TileSource>
#include <osgEarth/URI>

namespace FePlugin
{
    using namespace osgEarth;

    /**
    * @class QuadTreeOptions
    * @brief 四叉树配置选项类
    * @note 此类集成自TileSourceOptions，是平顶四叉树的TileSource的配置项类
    * @author c00005
    */
    class CFlatQuadTreeOptions : public TileSourceOptions
    {
    public:
        /**  
        * @note 返回earth文件配置的url
        */
        optional<URI>& url() { return m_urlFilePath; }

        /**  
        * @note 返回earth文件配置的url，此值不可被修改
        */
        const optional<URI>& url() const { return m_urlFilePath; }

        /**  
        * @note 返回该平顶四叉树的类型
        */
        optional<std::string>& quadtreeType() { return m_strQuadtreeType; }

        /**  
        * @note 返回该平顶四叉树的类型，此值不可被修改
        */
        const optional<std::string>& quadtreeType() const { return m_strQuadtreeType; }

        /**  
        * @note 返回该平顶四叉树的文件的格式
        */
        optional<std::string>& format() { return m_strFileFormat; }

        /**  
        * @note 返回该平顶四叉树的文件的格式，此值不可被修改
        */
        const optional<std::string>& format() const { return m_strFileFormat; }

    public:
        /**  
        * @brief 构造函数
        * @note 带有TileSourceOptions的构造函数，用于初始化平顶四叉树的配置项  
        * @param opt [in] TileSourceOptions的值
        * @return 无
        */
        CFlatQuadTreeOptions( const TileSourceOptions& opt =TileSourceOptions() ) : TileSourceOptions( opt )
        {
            setDriver( "flatgis" );
            fromConfig( _conf );
        }

        /**  
        * @brief 构造函数
        * @note 带有earth文件中配置的url的构造函数，用于初始化平顶四叉树的url  
        * @param inUrl [in] earth文件中配置的url的路径
        * @return 无
        */
        CFlatQuadTreeOptions( const std::string& inUrl ) : TileSourceOptions()
        {
            setDriver( "flatgis" );
            fromConfig( _conf );
            url() = inUrl;
        }

        /**  
        * @note 析构函数
        */
        virtual ~CFlatQuadTreeOptions() { }

    public:
        /**  
        * @brief 取得本四叉树的配置项
        * @note 首先取得父类的config，如果子类也设置了其中的配置项，则更新并返回
        * @return Config 返回本类的config
        */
        Config getConfig() const {
            Config conf = TileSourceOptions::getConfig();
            conf.updateIfSet("url", m_urlFilePath);
            conf.updateIfSet("quadtree_type", m_strQuadtreeType);
            conf.updateIfSet("format", m_strFileFormat);
            return conf;
        }

    protected:
        /**  
        * @brief 合并config
        * @note 将父类与子类的config进行合并  
        * @param conf [out] 将父类以及本类中的配置项输入到此变量中，并返回 
        * @return 无
        */
        void mergeConfig( const Config& conf ) {
            TileSourceOptions::mergeConfig( conf );
            fromConfig( conf );
        }

    private:
        /**  
        * @brief 取得config
        * @note 如果选项被配置了，则返回该配置项  
        * @param conf [out] 用于返回本类的配置项
        * @return 无
        */
        void fromConfig( const Config& conf ) {
            conf.getIfSet( "url", m_urlFilePath );
            conf.getIfSet( "format", m_strFileFormat );
            conf.getIfSet( "quadtree_type", m_strQuadtreeType );
        }

        ///保存earth文件中配置的url即索引文件的路径
        optional<URI>           m_urlFilePath;
        ///保存平顶四叉树的类型
        optional<std::string>   m_strQuadtreeType;
        ///保存文件的格式
        optional<std::string>   m_strFileFormat;
    };

} 

#endif

