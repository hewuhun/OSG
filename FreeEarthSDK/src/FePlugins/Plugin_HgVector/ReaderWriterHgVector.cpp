#include <HgVectorImageDataSource.h>

#include <ImageDecode.h>

#include <osgEarth/TileSource>
#include "osgDB/FileNameUtils"

using namespace osgEarth;

class CHgVectorTileSource : public TileSource
{
public:
	CHgVectorTileSource(const TileSourceOptions& options =TileSourceOptions()) : TileSource(options)
	{
		data_source = new CHgVectorDataSource();
	}

	~CHgVectorTileSource()
	{
		delete data_source;
	}

public:
	osgEarth::TileSource::Status initialize(const osgDB::Options* dbOptions)
	{
		if ( !getProfile() )
		{
			setProfile( Profile::create( "wgs84", "", 2, 1 ) );
		}

		return STATUS_OK;
	}

	osg::Image* createImage( const TileKey& key, ProgressCallback* progress )
	{
		unsigned int tile_x, tile_y;
		key.getTileXY(tile_x, tile_y);
		unsigned int lod = key.getLevelOfDetail();

		tile t;
		t.l = lod;
		t.x = tile_x;
		t.y = tile_y;
		t.tile_type = 3;

		tile_data t_d = data_source->GetTDTImageTile(t);

		if( t_d.size<= 0 )
		{
			//printf("该瓦片下载失败。\n");
			return 0;
		}

		if((t_d.size<200)&&(t_d.size>0))
		{
			//printf("一个损坏的PNG瓦片。内容: %s\n",t_d.data);
			delete [] t_d.data;

			return 0;
		}

		int w;
		int h;
		int c;

		unsigned char * data = LoadPNGFormBufRGBA((unsigned char *)t_d.data,t_d.size,w,h,c);

		if(!data)
		{
			//printf("PNG瓦片解码失败。内容: %s\n",t_d.data);
			delete [] t_d.data;
			return 0;
		}

		delete [] t_d.data;

		osg::Image* image =  new osg::Image();
		image->setImage(w,h,1,c,GL_RGBA,GL_UNSIGNED_BYTE,data,osg::Image::USE_NEW_DELETE);
		return image;
	}

protected:
	CHgVectorDataSource *data_source;
};

class CReaderWriterHgVectorFactory : public TileSourceDriver
{
public:
	/**  
	* @brief 构造函数
	* @note 默认构造函数，显示支持的读取文件的扩展名
	* @return 无
	*/
	CReaderWriterHgVectorFactory()
	{
		supportsExtension( "osgearth_hgvector", "HG Vector Service" );
	}

	/**  
	* @brief 返回类名
	* @note 返回读取文件的类名
	* @return char* 返回读取文件的类名
	*/
	virtual const char* className()
	{
		return "Hg Vector Service ReaderWriter";
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

		return new CHgVectorTileSource( getTileSourceOptions(options) );
	}
};

REGISTER_OSGPLUGIN(osgearth_hgvector, CReaderWriterHgVectorFactory)