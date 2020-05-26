#include "ReaderWriterWMTS.h"

#include <sstream>
#include <stdlib.h>
#include <memory.h>
#include <sstream>
#include <strstream> 

#include <osg/Image>
#include <osgEarth/URI>
#include <osgEarth/HTTPClient>
#include <osg/Shape>
#include <osgDB/FileNameUtils>
#include <osgEarth/Registry>

#define LC "[VBF driver] "

static int nIndex = 0;
static double dTotal = 0.0;

using namespace osgEarth;
//----------------------------------------------------------------------------------------
// 函数：CTileSourceWMTS()
// 描述：构造函数
//----------------------------------------------------------------------------------------
CTileSourceWMTS::CTileSourceWMTS(const TileSourceOptions& options) :
    TileSource( options ),
    m_options(options)
{

}

//----------------------------------------------------------------------------------------
// 函数：~CTileSourceWMTS()
// 描述：析构函数
//----------------------------------------------------------------------------------------
CTileSourceWMTS::~CTileSourceWMTS()
{

}

//----------------------------------------------------------------------------------------
// 函数：initialize()
// 描述：初始化
//----------------------------------------------------------------------------------------
TileSource::Status CTileSourceWMTS::initialize( const osgDB::Options* dbOptions)
{   
	if( !getProfile() )
    {
        setProfile( Registry::instance()->getGlobalGeodeticProfile() );
    }
	
    return STATUS_OK;
}

//----------------------------------------------------------------------------------------
// 函数：ConvertTileKeyToVBF()
// 描述：把osgEarth的地形块编码，转换为VBF的地形块编码
//----------------------------------------------------------------------------------------
void CTileSourceWMTS::ConvertTileKeyToVBF(const TileKey& key, int& nLod, int& nBlkNoX, int& nBlkNoY)
{
	//osgTerrain::TileID nID = key.getTileId();

	// 注意：osgEarth的纵向编号与VBF的编号上下相反
	unsigned tx, ty;
	getProfile()->getNumTiles(key.getLevelOfDetail(), tx, ty);

	nLod    = key.getLevelOfDetail();
	nBlkNoX = key.getTileX();
	nBlkNoY = ty - key.getTileY() - 1;
}

//----------------------------------------------------------------------------------------
// 函数：createImage()
// 描述：创建纹理
//----------------------------------------------------------------------------------------
osg::Image* CTileSourceWMTS::createImage(const TileKey& key, ProgressCallback* progress)
{
	//clock_t start, finish;   
	//double duration;   
	//start = clock();

	//URI uri = m_options.GetURL().value();
	//std::string strLayers = m_options.GetLayer().value();
	//std::string strSRS = m_options.GetSRS().value();
	//std::string strFormat = std::string("image/")+m_options.GetFormat().value();
	//int nLod = key.getTileId().level;
	//int nBlkNoX = key.getTileId().x;
	//int nBlkNoY = key.getTileId().y;

	//CVBF_HTTPRequest httpRequest(uri.full());
	//httpRequest.addParameter("request", "GetTile");
	//httpRequest.addParameter("version", "1.0.0");
	//httpRequest.addParameter("style", "");
	//httpRequest.addParameter("layer", strLayers);
	//httpRequest.addParameter("format", strFormat);
	//httpRequest.addParameter("TileMatrixSet", strSRS);

	//std::stringstream ssTileMatrix;
	//ssTileMatrix<<strSRS<<":"<<nLod;
	//httpRequest.addParameter("TileMatrix", ssTileMatrix.str());

	//httpRequest.addParameter("TileRow", nBlkNoX);
	//httpRequest.addParameter("TileCol", nBlkNoY);

	//ref_ptr<osg::Image> pImage = URI(httpRequest.getURL()).readImage().getImage();

	osg::ref_ptr<osg::Image> image;

	ReadResult response;
	osgDB::ReaderWriter* reader = fetchTileAndReader( key, progress, response );
	if ( reader )
	{
		std::istringstream buf( response.getString() );
		osgDB::ReaderWriter::ReadResult readResult = reader->readImage( buf, 0 );
		if ( readResult.error() )
		{
			//std::cout << "WMS: image read failed for " << createURI(key) << std::endl;
		}
		else
		{
			image = readResult.getImage();
		}
	}

	return image.release();
	//finish = clock();   
 	//duration = (double)(finish - start) / CLOCKS_PER_SEC; /*CLOCKS_PER_SEC，它用来表示一秒钟会有多少个时钟计时单元*/  
 	//printf( "%f seconds\n", duration );  
 	//dTotal+=duration;
 	//nIndex++;
 	//if (nIndex >= 100)
 	//{
 	//	std::cout<<"100次执行平均时间为："<<dTotal/100.0<<std::endl;
 	//	nIndex = 0;
 	//	dTotal = 0;
 	//}

	//if (!pImage.valid())
	//	return NULL;
	//return pImage.release();
}

inline float intBitsToFloat(int bits)
{
	union
	{
		int i;
		float f;
	} u;
	u.i = bits;
	return u.f;
}

//----------------------------------------------------------------------------------------
// 函数：createHeightField()
// 描述：创建高程
//----------------------------------------------------------------------------------------
osg::HeightField* CTileSourceWMTS::createHeightField(const TileKey& key, ProgressCallback* progress)
{
	URI uri = m_options.GetURL().value();
	std::string strLayers = m_options.GetLayer().value();
	std::string strSRS = m_options.GetSRS().value();
	std::string strFormat = std::string("Elevation/")+m_options.GetFormat().value();
	
	int nLod = key.getLevelOfDetail();
	int nBlkNoX = key.getTileX();
	int nBlkNoY = key.getTileX();

	HTTPRequest httpRequest(uri.full());
	httpRequest.addParameter("request", "GetElevation");
	httpRequest.addParameter("version", "1.0.0");
	httpRequest.addParameter("style", "");
	httpRequest.addParameter("layer", strLayers);
	httpRequest.addParameter("format", strFormat);
	httpRequest.addParameter("TileMatrixSet", strSRS);

	std::stringstream ssTileMatrix;
	ssTileMatrix<<strSRS<<":"<<nLod;
	httpRequest.addParameter("TileMatrix", ssTileMatrix.str());

	httpRequest.addParameter("TileRow", nBlkNoX);
	httpRequest.addParameter("TileCol", nBlkNoY);

	std::string strUrl = httpRequest.getURL();
	
	HTTPClient httpClient;
	httpClient.setTimeout(200);
	HTTPResponse httpResponse = httpClient.get(httpRequest);
	if(!httpResponse.isOK() || 
		httpResponse.getNumParts() == 0 ||
		httpResponse.getPartAsString(0).empty())
	{
		return NULL;
	}
	
	// 将字节流转换为float类型
	const int nDataSizeBytes = 17*17*4;
	const int nDemElmCount = nDataSizeBytes/sizeof(float);

	// 读取字节数据
	char* pByteData = new char[nDataSizeBytes];
	memset(pByteData, 0, nDataSizeBytes);
	httpResponse.getPartStream(0).read(pByteData, nDataSizeBytes);

	float* pDemData = new float[nDemElmCount];
	for(int n = 0, m = 0; m < nDemElmCount && n < nDataSizeBytes; m++)
	{
		int nData = 0;
		nData |= (pByteData[n++] << 3*8)&0xFF000000;
		nData |= (pByteData[n++] << 2*8)&0x00FF0000;
		nData |= (pByteData[n++] << 1*8)&0x0000FF00;
		nData |= (pByteData[n++])&0x000000FF;

		pDemData[m] = intBitsToFloat(nData);
	}
	delete pByteData;
	pByteData = NULL;


	// 创建osg::HeightField对象
	osg::ref_ptr<osg::HeightField> pHeightFiled = new osg::HeightField;
	pHeightFiled->allocate(17, 17);

	osg::FloatArray* pArray = pHeightFiled->getFloatArray();
	memcpy( (float*)pArray->getDataPointer(), pDemData, nDataSizeBytes);

	delete pDemData;

	return pHeightFiled.release();
}

osgDB::ReaderWriter* CTileSourceWMTS::fetchTileAndReader( 
	const TileKey& key, 
	ProgressCallback* progress, 
	ReadResult& out_response )
{
	osgDB::ReaderWriter* result = 0L;

	std::string uri = createURI(key);

	out_response = URI( uri ).readString( 0, progress );
	if ( out_response.succeeded() )
	{
		// get the mime type:
		std::string mt = out_response.metadata().value( IOMetadata::CONTENT_TYPE );

		if ( mt == "application/vnd.ogc.se_xml" || mt == "text/xml" || mt == "application/xml")
		{
			std::istringstream content( out_response.getString() );

			Config se;
			if( se.fromXML(content)  ) 
			{
				Config ex = se.child("serviceexceptionreport").child("serviceexception");
				if ( !ex.empty() )
				{
					OE_NOTICE << "WMTS Service Exception: " << ex.toJSON(true) << std::endl;
				}
				else
				{
					OE_NOTICE << "WMTS Response: " << se.toJSON(true) << std::endl;
				}
			}
			else
			{
				OE_NOTICE << "WMTS: unknown error." << std::endl;
			}
		}
		else
		{
			// really ought to use mime-type support here -GW
			std::string typeExt = mt.substr( mt.find_last_of("/")+1 );
			result = osgDB::Registry::instance()->getReaderWriterForExtension( typeExt );
			if ( !result )
			{
				OE_NOTICE << "WMTS: no reader registered; URI=" << createURI(key) << std::endl;
			}
		}
	}

	return result;
}

std::string CTileSourceWMTS::createURI( const TileKey& key ) const
{
	URI uri = m_options.GetURL().value();
	std::string strLayers = m_options.GetLayer().value();
	std::string strSRS = m_options.GetSRS().value();
	std::string strFormat = std::string("image/")+m_options.GetFormat().value();
	int nLod = key.getLevelOfDetail();
	int nBlkNoX = key.getTileY();
	int nBlkNoY = key.getTileX();

	HTTPRequest httpRequest(uri.full());
	httpRequest.addParameter("request", "GetTile");
	httpRequest.addParameter("version", "1.0.0");
	httpRequest.addParameter("style", "");
	httpRequest.addParameter("layer", strLayers);
	httpRequest.addParameter("format", strFormat);
	httpRequest.addParameter("TileMatrixSet", strSRS);

	std::stringstream ssTileMatrix;
	ssTileMatrix<<strSRS<<":"<<nLod;
	httpRequest.addParameter("TileMatrix", ssTileMatrix.str());

	httpRequest.addParameter("TileRow", nBlkNoX);
	httpRequest.addParameter("TileCol", nBlkNoY);

	return httpRequest.getURL();
}


CReaderWriterWMTS::CReaderWriterWMTS() 
{
    supportsExtension( "osgearth_wmts", "osgearth tile source driver WMTS" );
}

const char* CReaderWriterWMTS::className() const
{ 
    return "osgearth Tile ReaderWriter WMTS";
}

bool CReaderWriterWMTS::acceptsExtension(const std::string& extension) const
{
    return osgDB::equalCaseInsensitive( extension, "osgearth_wmts" );
}

osgDB::ReaderWriter::ReadResult CReaderWriterWMTS::readObject(const std::string& file_name, const Options* opt) const
{
	if( !acceptsExtension( osgDB::getLowerCaseFileExtension( file_name ) ) )
	{
		return ReadResult::FILE_NOT_HANDLED;
	}
	return new CTileSourceWMTS( getTileSourceOptions(opt) );
}

REGISTER_OSGPLUGIN(osgearth_wmts, CReaderWriterWMTS)
