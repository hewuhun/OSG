#include <FeLayers/FeatureReaderWriter.h>

namespace FeLayers
{
	CFeatureReaderWriter::CFeatureReaderWriter()
	{

	}

	CFeatureReaderWriter::~CFeatureReaderWriter()
	{

	}

	bool CFeatureReaderWriter::Read( const std::string& strFilePath, osgEarth::ConfigSet& configSets )
	{
		osgEarth::ReadResult r = osgEarth::URI(strFilePath).readString();
		if ( r.failed() )
			return false;

		std::stringstream in(r.getString());
		osg::ref_ptr<osgEarth::XmlDocument> doc = osgEarth::XmlDocument::load(in);
		if ( !doc.valid() )
			return false;

		osgEarth::Config docConf = doc->getConfig().child("layers");
		configSets = docConf.children( "layer" );

		return true;
	}
}