#include <viewPoint/ViewPointSerializer.h>

#include <osgEarth/XmlUtils>

#include <fstream>

#include <FeUtils/StrUtil.h>
#include "mainWindow/FreeUtil.h"

using namespace FeUtil;

namespace FreeViewer
{
	CViewPointReaderWriter::CViewPointReaderWriter()
	{

	}

	CViewPointReaderWriter::~CViewPointReaderWriter()
	{

	}

	FreeViewPoints CViewPointReaderWriter::Load( std::string strPath )
	{
		osg::ref_ptr<osgEarth::XmlDocument> pDoc = osgEarth::XmlDocument::load(strPath);
		if (pDoc.valid())
		{
			osgEarth::Config rootConfig = pDoc->getConfig();

			osgEarth::Config layersConfig = rootConfig.child("viewpoints");
			osgEarth::ConfigSet topLevelSet = layersConfig.children();
			osgEarth::ConfigSet::iterator levelItem = topLevelSet.begin();
			for (; levelItem != topLevelSet.end(); levelItem++)
			{
				double dLon=0.0, dLat=0.0, dHei=0.0, dHeading=0.0, dPitch=0.0, dRange=0.0, dTime=0.0;
				std::string strName = "";

				dLon = FeUtil::StringToDouble(levelItem->child("lon").value());
				dLat = FeUtil::StringToDouble(levelItem->child("lat").value());
				dHei = FeUtil::StringToDouble(levelItem->child("hei").value());
				dHeading = FeUtil::StringToDouble(levelItem->child("heading").value());
				dPitch = FeUtil::StringToDouble(levelItem->child("pitch").value());
				dRange = FeUtil::StringToDouble(levelItem->child("range").value());
				dTime = FeUtil::StringToDouble(levelItem->child("time").value());

#ifdef __QT4_USE_UNICODE
				strName = ConvertToCurrentEncoding(levelItem->child("name").value()).toStdString();
#else
				strName = levelItem->child("name").value();
#endif

				FeUtil::CFreeViewPoint point(strName, dLon, dLat, dHei, dHeading, dPitch, dRange, dTime);
				m_viewPoints.push_back(point);
			}
		}

		return m_viewPoints;
	}

	bool CViewPointReaderWriter::Save( std::string strPath )
	{
		osgEarth::Config rootConfig("viewpoints");

		FreeViewPoints::iterator it = m_viewPoints.begin();
		while (it != m_viewPoints.end())
		{
			osgEarth::Config config("viewpoint");
			config.add("lon", it->GetLon());
			config.add("lat", it->GetLat());
			config.add("hei", it->GetHei());
			config.add("heading", it->GetHeading());
			config.add("pitch", it->GetPitch());
			config.add("range", it->GetRange());
			config.add("time", it->GetTime());

#ifdef __QT4_USE_UNICODE
			std::string strName = ConvertToSDKEncoding(it->GetName().c_str());
			config.add("name", strName);
#else
			config.add("name", it->GetName());
#endif
			rootConfig.add(config);

			it++;
		}

		osgEarth::XmlDocument doc(rootConfig);

		std::ofstream out;
		out.open(strPath.c_str(), std::ios_base::trunc);
		doc.store(out);
		out.close();

		return true;
	}

	FreeViewPoints CViewPointReaderWriter::GetViewPoints()
	{
		return m_viewPoints;
	}

	void CViewPointReaderWriter::SetViewPoints( FreeViewPoints viewPoints )
	{
		m_viewPoints = viewPoints;
	}

	void CViewPointReaderWriter::AddViewPoint( CFreeViewPoint point )
	{
		m_viewPoints.push_back(point);
	}

}
