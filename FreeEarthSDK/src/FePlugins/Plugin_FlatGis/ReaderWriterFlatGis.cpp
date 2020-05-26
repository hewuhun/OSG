/**************************************************************************************************
* @file ReaderWriterFlagGis.h
* @note 读取平顶四叉树格式的影像高程
* @author c00005
* @data 2013-12-24
**************************************************************************************************/
#include <osgEarth/TileSource>
#include <osgEarth/FileUtils>
#include <osgEarth/ImageUtils>
#include <osgEarth/Registry>
#include <osgEarthUtil/TMS>
#include <osgEarth/HeightFieldUtils>

#include <osg/Notify>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <sstream>
#include <iomanip>
#include <string.h>

#include "FlatGisOptions.h"


using namespace osg;
using namespace osgEarth;
using namespace osgEarth::Util;

namespace FePlugin
{
    /**
    * @class CFlatQuadTreeSource
    * @brief 简要说明
    * @note 详细说明
    * @author c00005
    */
    class CFlatQuadTreeSource : public TileSource
    {
    public:
        /**  
        * @brief 构造函数
        * @note 带参的构造函数  
        * @param options [in] 传入的配置项，用于初始化父类以及本类的配置项
        * @return 无
        */
        CFlatQuadTreeSource(const TileSourceOptions& options)
            : TileSource(options),
            m_quadOptions(options)
        {
            m_bInvertY = m_quadOptions.quadtreeType() != "google";
        }

        /**  
        * @brief 初始化
        * @note 初始化CFlatQuadTreeSource这个TileSource
        * @param dbOptions [in] 
        * @return Status 是否初始化完成的状态
        */
        Status initialize(const osgDB::Options* dbOptions)
        {
            m_rpDbOptions = Registry::instance()->cloneOrCreateOptions(dbOptions);

            const Profile* profile = getProfile();

            //获取资源文件的url
            URI quadtreeURI = m_quadOptions.url().value();
            if ( quadtreeURI.empty() )
            {
                return Status::Error( "Fail: TMS driver requires a valid \"url\" property" );
            }

            //如果已经给出了一个profile，则使用重写的profile
            if ( profile )
            {
                //根据配置项中的信息创建一个TileMap
                m_rpTileMap = TMS::TileMap::create( 
                    m_quadOptions.url()->full(),
                    profile,
                    m_quadOptions.format().value(),
                    m_quadOptions.tileSize().value(), 
                    m_quadOptions.tileSize().value() );
            }
            else
            {
                //尝试从TMS的TileMap XML 中读取tilemap的参数
                m_rpTileMap = TMS::TileMapReaderWriter::read( 
                    quadtreeURI.full() + "/config.xml", m_rpDbOptions.get() );

                if (!m_rpTileMap.valid())
                {
                    return Status::Error( Stringify() << "Failed to read tilemap from "
                        << quadtreeURI.full() );
                }

                //通过TileMap创建一个空间信息配置项
                profile = m_rpTileMap->createProfile();
                if ( profile )
                    setProfile( profile );
            }

            //再次确保已经建立了一个空间信息配置项
            if ( !profile )
            {
                return Status::Error( Stringify() << "Failed to establish a profile for " 
                    << quadtreeURI.full() );
            }

            //TileMap和空间信息配置项已经有效，开始建立瓦片集合，自动的设置TileMap的最大最小显示级别
            if ( m_rpTileMap->getTileSets().size() > 0 )
            {
                if (m_rpTileMap->getDataExtents().size() > 0)
                {
                    for (DataExtentList::iterator itr = m_rpTileMap->getDataExtents().begin(); 
                        itr != m_rpTileMap->getDataExtents().end(); ++itr)
                    {
                        this->getDataExtents().push_back(*itr);
                    }
                }
                else
                {
                    this->getDataExtents().push_back(DataExtent(
                        profile->getExtent(), 0, m_rpTileMap->getMaxLevel()));
                }
            }

            CachePolicy::NO_CACHE.apply( m_rpDbOptions.get() );

            return STATUS_OK;
        }

        /**  
        * @brief 获取最后修改时间
        * @note 获取TileSource的最后修改时间
        * @return TimeStamp
        */
        TimeStamp getLastModifiedTime() const
        {
            if ( m_rpTileMap.valid() )
                return m_rpTileMap->getTimeStamp();
            else
                return TileSource::getLastModifiedTime();
        }

        /**  
        * @brief 创建缓存策略（暂时保留）
        * @note 根据源文件是本地或者网络的创建不同的缓存策略
        * @param targetProfile [in]
        * @return CachePolicy 返回缓存的策略
        */
        //CachePolicy getCachePolicyHint(const Profile* targetProfile) const
        //{
        //    //如果读取的资源是本地的数据，则不创建缓存，否则使用默认的缓存策略
        //    if (!m_quadOptions.url()->isRemote() &&
        //        targetProfile && 
        //        targetProfile->isEquivalentTo(getProfile()) )
        //    {
        //        return CachePolicy::NO_CACHE;
        //    }
        //    else
        //    {
        //        return CachePolicy::DEFAULT;
        //    }
        //}

        /**  
        * @brief 创建影像文件
        * @note 根据传入的key值，构造出url，从而读取影像文件
        * @param key [in] 传入key值，用于构造文件的url
        * @param progress [in]
        * @return osg::Image* 返回读取到的影像文件
        */
        osg::Image* createImage(const TileKey&        key,
            ProgressCallback*     progress )
        {
            std::string image_url;

            //如果初始化的TileMap是有效的，并且key的级别小于最大的显示级别
            if (m_rpTileMap.valid() /*&& key.getLevelOfDetail() <= m_rpTileMap->getMaxLevel() */) //BY Y00001不做这个判断了，这个判断要求XML中有tilemap中配置layer
            {
				if(m_rpTileMap->getTileType() == "hang")
				{
					std::stringstream buf;
					int level = key.getLevelOfDetail();
					unsigned int tile_x, tile_y;
					key.getTileXY( tile_x, tile_y );

					buf << m_quadOptions.url()->full()
						<< "/" << level+1
						<< "/" << tile_x
						<< "/" << tile_y << "." << m_rpTileMap->getFormat().getExtension();

					image_url = buf.str();
				}
				else
				{
					std::stringstream buf;
					int level = key.getLevelOfDetail();
					unsigned int tile_x, tile_y;
					key.getTileXY( tile_y, tile_x );

					//第0行是南极，因此要对行x进行颠倒，用极值去减,某一级别的总行数为1<<level

					tile_x = (1<<level) - tile_x;
					tile_x = tile_x-1;
					//std::cout << "key" << key.str() <<std::endl;
					buf << m_quadOptions.url()->full()
						<< "/" << level
						<< "/" << tile_x
						<< "/" << tile_x << "_" << tile_y << "." << m_rpTileMap->getFormat().getExtension();

					image_url = buf.str();
				}
                //if(m_rpTileMap->getTileType() == "tms_standard_hang")
                //{     
                //    //通过TileMap进行构建读取图片的url
                //    std::stringstream buf;
                //    int level = key.getLevelOfDetail();
                //    unsigned int tile_x, tile_y;
                //    key.getTileXY( tile_y, tile_x );

                //    //第一步，级别+1
                //    level = level +1;
                //    //第二步，x, y颠倒，变为以行主序，当前就是以行主序的标准了
                //    //第三步，x 跳过 当前总数/4

                //    if(level <2)
                //    {
                //        int temp = tile_y;
                //        tile_y = tile_x;
                //        tile_x = temp;
                //    }
                //    else
                //    {
                //        tile_x = tile_x + (1<<(level-2));
                //    }

                //
                //    //std::cout << "key" << key.str() <<std::endl;
                //    buf << m_quadOptions.url()->full()
                //        << "/" << level
                //        << "/" << tile_x
                //        << "/" << tile_y << ".jpeg";

                //    image_url = buf.str();
                //}
                //else if(m_rpTileMap->getTileType() == "ww")
                //{     
                    //通过TileMap进行构建读取图片的url
                    //std::stringstream buf;
                    //int level = key.getLevelOfDetail();
                    //unsigned int tile_x, tile_y;
                    //key.getTileXY( tile_y, tile_x );

                    ////第0行是南极，因此要对行x进行颠倒，用极值去减,某一级别的总行数为1<<level

                    //tile_x = (1<<level) - tile_x;
                    //tile_x = tile_x-1;
                    ////std::cout << "key" << key.str() <<std::endl;
                    //buf << m_quadOptions.url()->full()
                    //    << "/" << level
                    //    << "/" << tile_x
                    //    << "/" << tile_x << "_" << tile_y << "." << m_rpTileMap->getFormat().getExtension();

                    //image_url = buf.str();
                    //std::cout << image_url << std::endl;
                //}
                //else
                //{
                //    image_url = m_rpTileMap->getURL( key,!m_bInvertY );
                //}

                //根据构建的url读取图片
                osg::ref_ptr<osg::Image> image;
                if (!image_url.empty())
                {
                    image = URI(image_url).readImage( m_rpDbOptions.get(), progress ).getImage();
                }

                if (!image.valid())
                {
                    if (image_url.empty() || !m_rpTileMap->intersectsKey(key))
                    {
                        /*如果不能从url或者缓存中读取到图片，检查给定的key是否小于tileMap中
                        最大的显示级别，则创建一个透明的图像*/
                        if (key.getLevelOfDetail() <= m_rpTileMap->getMaxLevel())
                        {
                            return ImageUtils::createEmptyImage();
                        }
                    }
                }
                return image.release();
            }
            return 0;
        }

        /**  
        * @brief 创建高程文件
        * @note 根据传入的key值，组合出url，从而读取高程信息
        * @param key [in] 传入key值，用于构造文件的url
        * @param progress [in]
        * @return osg::HeightField* 返回高程格式数据的指针
        */
        osg::HeightField* createHeightField( const TileKey& key,
            ProgressCallback* progress)
        {
            //如果初始化的TileMap是有效的，并且key的级别小于最大的显示级别
            if (m_rpTileMap.valid() /*&& key.getLevelOfDetail() <= m_rpTileMap->getMaxLevel()*/ )
            {
                //通过TileMap进行构建读取高程的url
                std::string hf_url = m_rpTileMap->getURL( key, m_bInvertY );
				

                //读取高程
                osg::ref_ptr<osg::HeightField> hf;
                if (!hf_url.empty())
                {
                    hf = dynamic_cast<osg::HeightField*>(URI(hf_url).readObject(
                        m_rpDbOptions.get(), progress ).getObject());
                    //hf = dynamic_cast<osg::HeightField*>(URI("C:/Users/Cuiwanfu/Desktop/0.osgb").readObject(
                    //    m_rpDbOptions.get(), progress ).getObject());
                }

                if (!hf.valid())
                {
                    if (hf_url.empty() || !m_rpTileMap->intersectsKey(key))
                    {
                        /*如果不能从url或者缓存中读取到图片，检查给定的key是否小于tileMap中
                        最大的显示级别，则创建一个透明的图像*/
                        if (key.getLevelOfDetail() <= m_rpTileMap->getMaxLevel())
                        {
                            return HeightFieldUtils::createReferenceHeightField(
                                key.getExtent(),  m_rpTileMap->getFormat().getWidth(),
                                m_rpTileMap->getFormat().getHeight());
                        }
                    }
                }
                return hf.release();
            }
            return 0;
        }

        /**  
        * @brief 返回瓦片的大小
        * @note 返回影像高程瓦片的每个片元的大小
        * @return int 返回片元的大小
        */
        virtual int getPixelsPerTile() const
        {
            return m_rpTileMap->getFormat().getWidth();
        }

        /**  
        * @brief 获取扩展名
        * @note 获取到TileMap中配置的文件的扩展名
        * @return std::string 返回文件的扩展名
        */
        virtual std::string getExtension()  const 
        {
            return m_rpTileMap->getFormat().getExtension();
        }

    private:
        ///初始化一个TMS的TileMap
        osg::ref_ptr<TMS::TileMap>   m_rpTileMap;
        ///判断是否反转Y的值
        bool                         m_bInvertY;
        ///平顶四叉树机制的配置项
        const CFlatQuadTreeOptions   m_quadOptions;
        ///读取插件的配置项
        osg::ref_ptr<osgDB::Options> m_rpDbOptions;
    };



    /**************************************************************************************************
    * @file ReaderWriterFlagGis.h
    * @note 读写平顶四叉树组织结构的高程影像
    * @author c00005
    * @data 2013-12-24
    **************************************************************************************************/
    class CReaderWriterFlagQuadTreeFactory : public TileSourceDriver
    {
    private:
        typedef std::map< std::string,osg::ref_ptr<TMS::TileMap> > TileMapCache;
        TileMapCache m_rpTileMapCache;

    public:
        /**  
        * @brief 构造函数
        * @note 默认构造函数，显示支持的读取文件的扩展名
        * @return 无
        */
        CReaderWriterFlagQuadTreeFactory()
        {
            supportsExtension( "osgearth_flatgis", "Tile Map Service" );
        }

        /**  
        * @brief 返回类名
        * @note 返回读取文件的类名
        * @return char* 返回读取文件的类名
        */
        virtual const char* className()
        {
            return "Tile Map Service ReaderWriter";
        }

        /**  
        * @brief 读取文件类
        * @note 判读是否支持读取某一格式的文件，如果是则构建一个TileSource并返回
        * @param file_name [in] 传入的需要读取的文件后缀名
        * @param options [in] 传入TileSource的配置项
        * @return ReadResult 返回构建的TileSource
        */
        virtual ReadResult readObject(const std::string& file_name, const Options* options) const
        {
            if ( !acceptsExtension(osgDB::getLowerCaseFileExtension( file_name )))
                return ReadResult::FILE_NOT_HANDLED;

            return new CFlatQuadTreeSource( getTileSourceOptions(options) );
        }
    };

    REGISTER_OSGPLUGIN(osgearth_flaggis, CReaderWriterFlagQuadTreeFactory)
}
