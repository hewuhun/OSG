#include <FeLayers/CustomLayerReaderWriter.h>

#include <FeUtils/PathRegistry.h>

//#include <fstream>
#include <osgDB/fstream>

namespace FeLayers
{
	CCustomLayerReaderWriter::CCustomLayerReaderWriter()
	{
		m_rootConfig.key() = "layers";
	}

	CCustomLayerReaderWriter::~CCustomLayerReaderWriter()
	{

	}

	osgEarth::Config CCustomLayerReaderWriter::Load( std::string strPath )
	{
		osg::ref_ptr<osgEarth::XmlDocument> pDoc = osgEarth::XmlDocument::load(strPath);
		if (pDoc.valid())
		{
			m_rootConfig = pDoc->getConfig();
		}

		return m_rootConfig;
	}

	bool CCustomLayerReaderWriter::Save( std::string strPath)
	{
		osgEarth::XmlDocument doc(m_rootConfig);
		
		osgDB::ofstream out;
        out.open(strPath.c_str(), std::ios_base::trunc);
		doc.store(out);
		out.close();

		return true;
	}

	void CCustomLayerReaderWriter::AppendConfig( osgEarth::Config config )
	{
		m_rootConfig.add(config);
	}

}
