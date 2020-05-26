#ifndef READER_WRITER_WMTS_H
#define READER_WRITER_WMTS_H

#include <osgDB/ReaderWriter>
#include <osgEarth/Progress>
#include <osgEarth/TileSource>

#include <TileSourceWMTSOptions.h>

using namespace osgEarth;

class CTileSourceWMTS : public TileSource
{
public:
	CTileSourceWMTS(const TileSourceOptions& options);

protected:
	virtual ~CTileSourceWMTS();

protected:
	// 初始化数据源
	virtual TileSource::Status initialize(const osgDB::Options* dbOptions);

	// 为地形块提供纹理数据	  
	virtual osg::Image* createImage(const TileKey& key,  ProgressCallback* progress);

	// 为地形块提供高程数据
	virtual osg::HeightField* createHeightField(const TileKey& key, ProgressCallback* progress);

protected:
	// 把osgEarth的地形块编码，转换为VBF的地形块编码
	void ConvertTileKeyToVBF(const TileKey& key, int& nLod, int& nBlkNoX, int& nBlkNoY);

	std::string createURI( const TileKey& key ) const;

	osgDB::ReaderWriter* fetchTileAndReader( 
		const TileKey&     key, 
		ProgressCallback*  progress, 
		ReadResult&        out_response );

private:
	const CTileSourceOptionsWMTS	m_options;
};

class CReaderWriterWMTS : public TileSourceDriver
{
public:
	CReaderWriterWMTS();

	virtual const char* className() const;
	virtual bool acceptsExtension(const std::string& extension) const;
	virtual osgDB::ReaderWriter::ReadResult readObject(const std::string& file_name, const Options* opt) const;
};

#endif

