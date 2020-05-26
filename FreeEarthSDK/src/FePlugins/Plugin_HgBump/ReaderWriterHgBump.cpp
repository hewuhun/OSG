/**************************************************************************************************
* @file ReaderWriterHgBump.h
* @note 读写影像文件
* @author c00005
* @data 2013-12-19
**************************************************************************************************/

#include <osgEarth/HeightFieldUtils>

#include <PluginDependent/HgSharpTileSource.h>

namespace FePlugin
{
    using namespace osgEarth;
    /**
    * @class HgGisSource
    * @brief 读写影像的类
    * @note 继承于TileSource，是特殊的一种影像的读写插件。
    * @author c00005
    */
    class CHgBumpSource : public CHgSharpTileSource
    {
    public:
        /**  
        * @brief 构造函数
        * @note 有参数的构造函数
        * @param options [in] 用于初始化本类中的options
        * @return 无
        */
        CHgBumpSource( const TileSourceOptions& options )
            :CHgSharpTileSource(options)
        {

        }

        /**  
        * @brief 创建影像
        * @note 根据传入的TileKye，分别解析出级数、行、列等信息，读取影像文件，重写父类方法
        * @param key [in] 通过key解析出级数、行、列，从而拼凑出影像文件所在位置
        * @param progress [in/out/inout]
        * @return osg::Image* 返回的是影像文件的指针
        */
        osg::Image* createImage(const TileKey& key, ProgressCallback* progress)
        {
			//先判断类型
			if(m_hgGisService.GetTileType() == "NormalImage")
			{
				std::string strUrl = m_hgGisService.ConstructUrl(key, m_hgOptions);

				//m_hgDbOptions->setOptionString("Compressor=zlib");
				//根据构建的url读取图片
				osg::ref_ptr<osg::Image> image;
				if (!strUrl.empty())
				{
					image = URI(strUrl).readImage( m_hgDbOptions, progress ).getImage();
				}

				////使用上面构造的存放到buf中的影像的路径等信息读取影像文件，并返回
				//osg::ref_ptr<osg::Image> hf;
				//if (!strUrl.empty())
				//{
				//	hf = dynamic_cast<osg::Image*>(osgDB::Registry::instance()->getReaderWriterForExtension( "osgb" )->readImage( strUrl, new osgDB::Options("Compressor=zlib")).getImage());
				//}
				return image.release();
			}

            //std::cout << key.str()<<std::endl;

            //获取单个像素的分辩率
            double res = 156543.03/4;

            //if(key.getLOD() < 5)
            //{
            //    res = res/3;
            //}

            double beishu = 3.0;

            for(unsigned int aa = 0; aa<key.getLOD(); aa++)
            {
                if(key.getLOD() <= 2)
                {
                    beishu = 4.0;
                    res = res/beishu;
                    break;
                }

                if(aa < 4)
                {
                    beishu = beishu-0.2;
                }
                else if((aa >= 4) && ((aa <= 6)))
                {
                    beishu = beishu-0.25;
                }
                else if((aa > 6) && ((aa <= 8)))
                {
                    beishu = beishu-0.001;
                }
                else if((aa > 9))
                {
                    if(beishu <= 1.0)
                    beishu = 1.0;
                }

                
                if(beishu <= 1)
                {
                    beishu = 1.0;
                }

                res = res/beishu;
            }

            osg::ref_ptr<osg::HeightField> hf = createHeightField(key, progress);
            if(hf.valid() == false)
            {
                return NULL;
            }

			int nDeaultSize = 256; //默认空白高程片的行列值（常见为正方形）
            osg::ref_ptr<osg::HeightField> leftHF = createHeightField(key.createNeighborKey(-1, 0), progress);
            if(leftHF.valid() == false)
            {
                //创建空白
                leftHF = HeightFieldUtils::createReferenceHeightField( key.getExtent(), nDeaultSize, nDeaultSize );
            }

            osg::ref_ptr<osg::HeightField> rightHF = createHeightField(key.createNeighborKey(1, 0), progress);
            if(rightHF.valid() == false)
            {
                //创建空白
                rightHF = HeightFieldUtils::createReferenceHeightField( key.getExtent(), nDeaultSize, nDeaultSize );
            }

            osg::ref_ptr<osg::HeightField> topHF = createHeightField(key.createNeighborKey(0, -1), progress);
            if(topHF.valid() == false)
            {
                //创建空白
                topHF = HeightFieldUtils::createReferenceHeightField( key.getExtent(), nDeaultSize, nDeaultSize );
            }

            osg::ref_ptr<osg::HeightField> buttomHF = createHeightField(key.createNeighborKey(0, 1), progress);
            if(buttomHF.valid() == false)
            {
                //创建空白
                buttomHF = HeightFieldUtils::createReferenceHeightField( key.getExtent(), nDeaultSize, nDeaultSize );
            }

			int sizet = m_hgGisService.GetTileInfo().GetTileSize();
            osg::ref_ptr<osg::Image> image = new osg::Image;
            image->allocateImage( sizet, sizet, 1, GL_RGBA, GL_UNSIGNED_BYTE);
            image->setInternalTextureFormat(GL_RGBA);

            for(int i = 0; i<sizet; i++)
            {
                for(int j = 0; j<sizet; j++)
                {

                    int Hl, Hr, Ht, Hb;

                    float sclae = 1.0;

                    if(i == 0)
                    {
                        Hl = leftHF->getHeight(sizet-1, j)*sclae; //最右
                    }
                    else
                    {
                        Hl = hf->getHeight(i-1, j)*sclae;
                    }


                    if(i == (sizet -1))
                    {
                        Hr = rightHF->getHeight(0, j)*sclae; //最右
                    }
                    else
                    {
                        Hr = hf->getHeight(i+1, j)*sclae;
                    }

                    if(j == (sizet-1))
                    {
                        Ht = topHF->getHeight(i, 0)*sclae;
                    }
                    else
                    {
                        Ht = hf->getHeight(i, j+1)*sclae;
                    }

                    if(j == 0)
                    {
                        Hb = buttomHF->getHeight(i, sizet-1)*sclae;
                    }
                    else
                    {
                        Hb = hf->getHeight(i, j-1)*sclae;
                    }



                    osg::Vec3 left = osg::Vec3(-res, 0, Hl - Hr);
                    osg::Vec3 top = osg::Vec3(0, -res, Hb - Ht);

                    osg::Vec3 normal = left ^ top;

                    normal.normalize();

                    unsigned char r = (normal.x()+1)*127.5;
                    unsigned char g = (normal.y()+1)*127.5;
                    unsigned char b = (normal.z()+1)*127.5;


                    *(((unsigned char*)image->data(i, j)) + 0) = r;
                    *(((unsigned char*)image->data(i, j)) + 1) = g;
                    *(((unsigned char*)image->data(i, j)) + 2) = b;
                    *(((unsigned char*)image->data(i, j)) + 3) = 0;
                }
            }
			image->setInternalTextureFormat( GL_RGBA8 );
			//osgDB::Registry::instance()->getReaderWriterForExtension( "osgb" )->writeImage(*(image.get()), "D:/abc/a.osgb", new osgDB::Options("Compressor=zlib"));
            return image.release();
        }

        /**  
        * @brief 创建高程
        * @note 根据传入参数key创建高程，重写父类方法
        * @param key [in] 
        * @param progress [in]
        * @return HeightField的空指针
        */
        osg::HeightField* createHeightField( const TileKey& key,
            ProgressCallback* progress)
        {
            std::string strUrl = m_hgGisService.ConstructUrl(key, m_hgOptions);

            //使用上面构造的存放到buf中的影像的路径等信息读取影像文件，并返回
            osg::ref_ptr<osg::HeightField> hf;
            if (!strUrl.empty())
            {
                hf = dynamic_cast<osg::HeightField*>(URI(strUrl).readObject( 
                    m_hgDbOptions.get(), progress ).getObject());
            }

            return hf.release();
        }

    };

    /**
    * @class HgGISTileSourceFactory
    * @brief 构造HgGisTileSource的工厂类
    * @note 继承自TileSourceDriver，是HgGis构造TileSource的工厂类，
    * @note 本类定义了支持的读取影像插件格式以及类
    * @author c00005
    */
    class CHgBUMPTileSourceFactory : public TileSourceDriver
    {
    public:
        /**  
        * @brief 构造函数
        * @note 本类的默认构造函数，构造了支持读取影像的插件
        * @return 无
        */
        CHgBUMPTileSourceFactory()
        {
            supportsExtension( "osgearth_hgbump", "HgBUMP Server" );
        }

        /**  
        * @brief 返回类名
        * @note 返回类名
        * @return const char* 返回常量字符指针
        */
        virtual const char* className()
        {
            return "HgBUMP Server REST ReaderWriter";
        }


        /**  
        * @brief 获取读取文件的插件
        * @note 根据传入的文件名获取到文件的扩展名，并判断该是否支持读取该文件，
        * @note 如果可以返回插件类名，否则返回不能处理
        * @param file_name [in] 传入文件的名称，通过该名称判断是否可以读取
        * @param options [in] 如果文件可以被读取，则将全局的options传入进来，供插件的构造使用
        * @return ReadResult 用于存放返回的结果，可能是无法处理，也可以是插件的类指针
        */
        virtual ReadResult readObject(const std::string& file_name, const Options* options) const
        {
            if ( !acceptsExtension(osgDB::getLowerCaseFileExtension( file_name )))
                return ReadResult::FILE_NOT_HANDLED;

            return new CHgBumpSource( getTileSourceOptions(options) );
        }
    };

    REGISTER_OSGPLUGIN(osgearth_hgbump, CHgBUMPTileSourceFactory)

}
