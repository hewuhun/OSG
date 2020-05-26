#include <FeExtNode/ExTiltReaderWriter.h>

#include <FeUtils/logger/LoggerDef.h>
#include <FeServiceProvider/CurlHelper.h>

#include <osgEarth/StringUtils>

namespace FeExtNode
{
	CExTiltConfig::CExTiltConfig()
	{
		m_vecScale = osg::Vec3d(1.0, 1.0, 1.0);
	}

	CExTiltConfig::~CExTiltConfig()
	{

	}
}

namespace FeExtNode
{
	using namespace FeUtil;
	using namespace FeUtil::TinyUtil;

	CExTiltReaderWriter::CExTiltReaderWriter()
		:TiXmlVisitor()
	{

	}

	CExTiltReaderWriter::CExTiltReaderWriter(bool bService)
		:TiXmlVisitor()
		,m_bService(bService)
	{

	}

	CExTiltReaderWriter::~CExTiltReaderWriter()
	{

	}

	bool CExTiltReaderWriter::Read( const std::string& strFilePath, CExTiltConfig& config )
	{
		m_strTiltPath = strFilePath;

		FeUtil::TiXmlDocument document (m_strTiltPath.c_str());
		if ( !document.LoadFile(FeUtil::TIXML_ENCODING_UTF8) || !document.Accept(this))
		{
			LOG(LOG_ERROR)<<"Can't load or parser tilt config file.";
			return false;
		}

		config = m_tempConfig;

		return true;
	}

	bool CExTiltReaderWriter::ReadURl( const std::string& strFilePath, CExTiltConfig& config )
	{
		FeServiceProvider::CurlHelper *pCurlHelper = new FeServiceProvider::CurlHelper();
		m_strTiltPath = strFilePath;
		std::string strXml;
		if (!pCurlHelper->GetReq(strFilePath, strXml))
		{
			return false;
		}
		
		FeUtil::TiXmlDocument document ;
		document.Parse(strXml.c_str());
		if (!document.Accept(this))
		{
			LOG(LOG_ERROR)<<"Can't load or parser tilt config file.";
			return false;
		}

		config = m_tempConfig;

		return true;
	}

	bool CExTiltReaderWriter::Write( const std::string& strFilePath, CExTiltConfig& config )
	{
#ifdef WIN32
        std::string strValue = "";
        char chValue[100];
        FeUtil::TiXmlDeclaration* declar = new FeUtil::TiXmlDeclaration("1.0", "UTF-8", "");
        FeUtil::TiXmlDocument* doc = new FeUtil::TiXmlDocument(strFilePath.c_str());
        doc->LinkEndChild(declar);

        FeUtil::TiXmlElement * ele = new FeUtil::TiXmlElement("tilt");
        doc->LinkEndChild(ele);

        //名称
        FeUtil::TiXmlElement * elname = new FeUtil::TiXmlElement("name");
        elname->SetAttribute("name",config.name().c_str());
        ele->LinkEndChild(elname);

		//数据加载类型
		FeUtil::TiXmlElement * elType = new FeUtil::TiXmlElement("type");
		elType->SetAttribute("type", config.tileType());

        //位置
        FeUtil::TiXmlElement * elPos = new FeUtil::TiXmlElement("position");
        std::sprintf(chValue,"%.10f",config.position().x());
        strValue = chValue;
        elPos->SetAttribute("lon",strValue.c_str());

        std::sprintf(chValue,"%.10f",config.position().y());
        strValue = chValue;
        elPos->SetAttribute("lat",strValue.c_str());

        std::sprintf(chValue,"%.10f",config.position().z());
        strValue = chValue;
        elPos->SetAttribute("hei",strValue.c_str());
        ele->LinkEndChild(elPos);

        //偏移量
        FeUtil::TiXmlElement * eloff = new FeUtil::TiXmlElement("offset");
        std::sprintf(chValue,"%.10f",config.offset().x());
        strValue = chValue;
        eloff->SetAttribute("lon",strValue.c_str());

        std::sprintf(chValue,"%.10f",config.offset().y());
        strValue = chValue;
        eloff->SetAttribute("lat",strValue.c_str());

        std::sprintf(chValue,"%.10f",config.offset().z());
        strValue = chValue;
        eloff->SetAttribute("hei",strValue.c_str());
        ele->LinkEndChild(eloff);

        //旋转
        FeUtil::TiXmlElement * elrot = new FeUtil::TiXmlElement("rotate");
        std::sprintf(chValue,"%.10f",config.rotate().x());
        strValue = chValue;
        elrot->SetAttribute("lon",strValue.c_str());

        std::sprintf(chValue,"%.10f",config.rotate().y());
        strValue = chValue;
        elrot->SetAttribute("lat",strValue.c_str());

        std::sprintf(chValue,"%.10f",config.rotate().z());
        strValue = chValue;
        elrot->SetAttribute("hei",strValue.c_str());
        ele->LinkEndChild(elrot);

        //缩放
        FeUtil::TiXmlElement * elscale = new FeUtil::TiXmlElement("scale");
        std::sprintf(chValue,"%.10f",config.scale().x());
        strValue = chValue;
        elscale->SetAttribute("lon",strValue.c_str());

        std::sprintf(chValue,"%.10f",config.scale().y());
        strValue = chValue;
        elscale->SetAttribute("lat",strValue.c_str());

        std::sprintf(chValue,"%.10f",config.scale().z());
        strValue = chValue;
        elscale->SetAttribute("hei",strValue.c_str());
        ele->LinkEndChild(elscale);

        FeUtil::TiXmlElement * elnodelist = new FeUtil::TiXmlElement("nodelist");
        std::vector<std::string> nodelist =  config.tiltList();
        std::vector<std::string>::iterator iter =  nodelist.begin();
        while( iter != nodelist.end())
        {
            FeUtil::TiXmlElement * elnode = new FeUtil::TiXmlElement("node");
            strValue = *iter;
            elnode->SetAttribute("node",strValue.c_str());
            elnodelist->LinkEndChild(elnode);

            iter++;
        }
        ele->LinkEndChild(elnodelist);

        return doc->SaveFile();
#endif
        return true;
	}

	bool CExTiltReaderWriter::VisitEnter( const TiXmlElement& ele , const TiXmlAttribute* attr )
	{
		if (AcceptKey("name", ele.Value()))
		{
			return AcceptHgtName(ele, attr);
		}

		if (AcceptKey("Name", ele.Value()))
		{
			return AcceptLfpName(ele, attr);
		}

		if (AcceptKey("position", ele.Value()))
		{
			return AcceptHgtPosition(ele, attr);
		}

		if (AcceptKey("Position", ele.Value()))
		{
			return AcceptLfpPosition(ele, attr);
		}

		if (AcceptKey("sml:Position", ele.Value()))
		{
			return AcceptScpPosition(ele, attr);
		}

		if (AcceptKey("offset", ele.Value()))
		{
			return AcceptHgtOffset(ele, attr);
		}

		if (AcceptKey("OffsetMeters", ele.Value()))
		{
			return AcceptLfpOffset(ele, attr);
		}

		if (AcceptKey("rotate", ele.Value()))
		{
			return AcceptHgtRotate(ele, attr);
		}

		if (AcceptKey("Rotation", ele.Value()))
		{
			return AcceptLfpRotate(ele, attr);
		}

		if (AcceptKey("scale", ele.Value()))
		{
			return AcceptHgtScale(ele, attr);
		}

		if (AcceptKey("Scale", ele.Value()))
		{
			return AcceptLfpScale(ele, attr);
		}

		if (AcceptKey("nodelist", ele.Value()))
		{
			return AcceptHgtNodeList(ele, attr);
		}

		if (AcceptKey("NodeList", ele.Value()))
		{
			return AcceptLfpNodeList(ele, attr);
		}

		if (AcceptKey("sml:OSGFiles", ele.Value()))
		{
			return AcceptScpNodeList(ele, attr);
		}

		if (AcceptKey("type", ele.Value()))
		{
			return AcceptTypeName(ele, attr);      
		}
		return true;
	}

	bool CExTiltReaderWriter::AcceptHgtName( const TiXmlElement& ele, const TiXmlAttribute* attr )
	{
		m_tempConfig.name() = ToStdString(ele.Attribute("name"));

		return true;
	}

	bool CExTiltReaderWriter::AcceptLfpName( const TiXmlElement& ele, const TiXmlAttribute* attr )
	{
		m_tempConfig.name() = ToStdString(ele.GetText());

		return true;
	}

	bool CExTiltReaderWriter::AcceptHgtPosition( const TiXmlElement& ele, const TiXmlAttribute* attr )
	{
		osg::Vec3d vecPos;
		vecPos.x() = osgEarth::as<double>(ToStdString(ele.Attribute("lon")),0.0);
		vecPos.y() = osgEarth::as<double>(ToStdString(ele.Attribute("lat")),0.0);
		vecPos.z() = osgEarth::as<double>(ToStdString(ele.Attribute("hei")),0.0);

		m_tempConfig.position() = vecPos;

		return true;
	}

	bool CExTiltReaderWriter::AcceptLfpPosition( const TiXmlElement& ele, const TiXmlAttribute* attr)
	{
		std::string strPosition = ToStdString(ele.GetText());
		int nLen = strPosition.length();
		char* cStr;//存放原字符串
		const char * cSplit = ","; //分隔符
		char * cP;//存放分割字符串
		cStr = (char *)malloc((nLen+1)*sizeof(char));
		strPosition.copy(cStr, nLen, 0);//转换格式
		cP = strtok (cStr,cSplit);//分割字符串
		std::string strPosx;
		std::string strPosy;
		std::string strPosz;
		for(int i = 1; cP!=NULL; ++i)
		{ 
			if (i == 1) strPosx = cP;
			else if (i == 2) strPosy = cP;
			else  strPosz = cP;
			cP = strtok(NULL,cSplit);
		}
		osg::Vec3d vecPos;
		vecPos.x()  = osgEarth::as<double>(strPosx,0.0);
		vecPos.y() = osgEarth::as<double>(strPosy,0.0);
		vecPos.z() = osgEarth::as<double>(strPosz,0.0);
		m_tempConfig.position() = vecPos;
		return true;
	}

	bool CExTiltReaderWriter::AcceptScpPosition( const TiXmlElement& ele, const TiXmlAttribute* attr )
	{
		std::string strPosx;
		std::string strPosy;
		std::string strPosz;

		const FeUtil::TiXmlElement* pChildModel = ele.FirstChildElement();
		while(pChildModel)
		{
			std::string strChildValue =  pChildModel->Value();
			if (0 == strChildValue.compare("sml:X"))
			{
				strPosx = pChildModel->GetText();
			}

			if (0 == strChildValue.compare("sml:Y"))
			{
				strPosy = pChildModel->GetText();
			}

			if (0 == strChildValue.compare("sml:Z"))
			{
				strPosz = pChildModel->GetText();
			}

			pChildModel = pChildModel->NextSiblingElement();
		}
		osg::Vec3d vecPos;
		vecPos.x()  = osgEarth::as<double>(strPosx,0.0);
		vecPos.y() = osgEarth::as<double>(strPosy,0.0);
		vecPos.z() = osgEarth::as<double>(strPosz,0.0);
		m_tempConfig.position() = vecPos;

		return true;
	}

	bool CExTiltReaderWriter::AcceptHgtOffset( const TiXmlElement& ele, const TiXmlAttribute* attr )
	{
		osg::Vec3d vecOffset;
		vecOffset.x() = osgEarth::as<double>(ToStdString(ele.Attribute("lon")),0.0);
		vecOffset.y() = osgEarth::as<double>(ToStdString(ele.Attribute("lat")),0.0);
		vecOffset.z() = osgEarth::as<double>(ToStdString(ele.Attribute("hei")),0.0);

		m_tempConfig.offset() = vecOffset;

		return true;
	}

	bool CExTiltReaderWriter::AcceptLfpOffset( const TiXmlElement& ele, const TiXmlAttribute* attr)
	{
		std::string strOffset = ToStdString(ele.GetText());
		int nLen = strOffset.length();
		char* cStr;//存放原字符串
		const char * cSplit = ","; //分隔符
		char * cP;//存放分割字符串
		cStr = (char *)malloc((nLen+1)*sizeof(char));
		strOffset.copy(cStr, nLen, 0);//转换格式
		cP = strtok (cStr,cSplit);//分割字符串
		std::string strOffx;
		std::string strOffy;
		std::string strOffz;
		for(int i = 1; cP!=NULL; ++i)
		{ 
			if (i == 1) strOffx = cP;
			else if (i == 2) strOffy = cP;
			else  strOffz = cP;
			cP = strtok(NULL,cSplit);
		}
		osg::Vec3d vecOffset;
		vecOffset.x() = osgEarth::as<double>(strOffx,0.0);
		vecOffset.y() = osgEarth::as<double>(strOffy,0.0);
		vecOffset.z() = osgEarth::as<double>(strOffz,0.0);
		m_tempConfig.offset() = vecOffset;
		return true;
	}

	bool CExTiltReaderWriter::AcceptHgtRotate( const TiXmlElement& ele, const TiXmlAttribute* attr )
	{
		osg::Vec3d vecRotate;
		vecRotate.x() = osgEarth::as<double>(ToStdString(ele.Attribute("x")),0.0);
		vecRotate.y() = osgEarth::as<double>(ToStdString(ele.Attribute("y")),0.0);
		vecRotate.z() = osgEarth::as<double>(ToStdString(ele.Attribute("z")),0.0);

		m_tempConfig.rotate() = vecRotate;
		return true;
	}

	bool CExTiltReaderWriter::AcceptLfpRotate( const TiXmlElement& ele, const TiXmlAttribute* attr )
	{
		std::string strRotate = ToStdString(ele.GetText());
		int nLen = strRotate.length();
		char* cStr;//存放原字符串
		const char * cSplit = ","; //分隔符
		char * cP;//存放分割字符串
		cStr = (char *)malloc((nLen+1)*sizeof(char));
		strRotate.copy(cStr, nLen, 0);//转换格式
		cP = strtok (cStr,cSplit);//分割字符串
		std::string strRotx;
		std::string strRoty;
		std::string strRotz;
		for(int i = 1; cP!=NULL; ++i)
		{ 
			if (i == 1) strRotx = cP;
			else if (i == 2) strRoty = cP;
			else  strRotz = cP;
			cP = strtok(NULL,cSplit);
		}
		osg::Vec3d vecRotate;
		vecRotate.x() = osgEarth::as<double>(strRotx,0.0);
		vecRotate.y() = osgEarth::as<double>(strRoty,0.0);
		vecRotate.z() = osgEarth::as<double>(strRotz,0.0);
		m_tempConfig.rotate() = vecRotate;
		return true;
	}

	bool CExTiltReaderWriter::AcceptHgtScale( const TiXmlElement& ele, const TiXmlAttribute* attr )
	{
		osg::Vec3d vecScale;
		vecScale.x() = osgEarth::as<double>(ToStdString(ele.Attribute("x")),0.0);
		vecScale.y() = osgEarth::as<double>(ToStdString(ele.Attribute("y")),0.0);
		vecScale.z() = osgEarth::as<double>(ToStdString(ele.Attribute("z")),0.0);

		m_tempConfig.scale() = vecScale;
		return true;
	}

	bool CExTiltReaderWriter::AcceptLfpScale( const TiXmlElement& ele, const TiXmlAttribute* attr)
	{
		std::string strScale = ToStdString(ele.GetText());
		int nLen = strScale.length();
		char* cStr;//存放原字符串
		const char * cSplit = ","; //分隔符
		char * cP;//存放分割字符串
		cStr = (char *)malloc((nLen+1)*sizeof(char));
		strScale.copy(cStr, nLen, 0);//转换格式
		cP = strtok (cStr,cSplit);//分割字符串
		std::string strScax;
		std::string strScay;
		std::string strScaz;
		for(int i = 1; cP!=NULL; ++i)
		{ 
			if (i == 1) strScax = cP;
			else if (i == 2) strScay = cP;
			else  strScaz = cP;
			cP = strtok(NULL,cSplit);
		}
		osg::Vec3d vecScale;
		vecScale.x() = osgEarth::as<double>(strScax,0.0);
		vecScale.y() = osgEarth::as<double>(strScay,0.0);
		vecScale.z() = osgEarth::as<double>(strScaz,0.0);
		m_tempConfig.scale() = vecScale;
		return true;
	}

	bool CExTiltReaderWriter::AcceptHgtNodeList( const TiXmlElement& ele, const TiXmlAttribute* attr )
	{
		//提取配置文件的所在路径
		std::string strFullPath = m_strTiltPath;
		unsigned int nSzp = strFullPath.rfind("/");
		std::string strPath = "";
		if (nSzp != std::string::npos)
		{
			strPath.assign(strFullPath, 0, nSzp + 1);
		}

		const FeUtil::TiXmlElement* pChildModel = ele.FirstChildElement(ele.FirstChild()->Value());
		while (pChildModel)
		{
			std::string strLocalPath = ToStdString(pChildModel->Attribute("node"));
            std::size_t szL = strLocalPath.find("/");
			std::string strTmp = "";
			std::string filePath = "";
			filePath = strPath;
			if (szL != std::string::npos)
			{
				strTmp.assign(strLocalPath,szL + 1,strLocalPath.length());
			}

			//如果不是服务 拼出完整路径
			if (!m_bService)
			{
				filePath.append(strTmp);
			}
			else
			{
				filePath = strLocalPath;
			}
			m_tempConfig.tiltList().push_back(filePath);

			pChildModel = pChildModel->NextSiblingElement(ele.FirstChild()->Value());
		}

		return true;
	}

	bool CExTiltReaderWriter::AcceptLfpNodeList( const TiXmlElement& ele, const TiXmlAttribute* attr)
	{
		//提取配置文件的所在路径
		std::string strFullPath = m_strTiltPath;
		unsigned int nSzp = strFullPath.rfind("/");
		std::string strPath = "";
		if (nSzp != std::string::npos)
		{
			strPath.assign(strFullPath, 0, nSzp + 1);
		}

		const FeUtil::TiXmlElement* pChildModel = ele.FirstChildElement(ele.FirstChild()->Value());
		while (pChildModel)
		{
			std::string strLocalPath = ToStdString(pChildModel->GetText());
			std::size_t szL = strLocalPath.find("/");
			std::string strTmp = "";
			std::string filePath = "";
			filePath = strPath;
			if (szL != std::string::npos)
			{
				strTmp.assign(strLocalPath,szL + 1,strLocalPath.length());
			}

			if (!m_bService)
			{
				filePath.append(strTmp);
			}
			else
			{
				filePath = strLocalPath;
			}
			m_tempConfig.tiltList().push_back(filePath);

			pChildModel = pChildModel->NextSiblingElement(ele.FirstChild()->Value());
		}

		return true;
	}

	bool CExTiltReaderWriter::AcceptScpNodeList( const TiXmlElement& ele, const TiXmlAttribute* attr )
	{
		//提取配置文件的所在路径
		std::string strFullPath = m_strTiltPath;
		unsigned int nSzp = strFullPath.rfind("/");
		std::string strPath = "";
		if (nSzp != std::string::npos)
		{
			strPath.assign(strFullPath, 0, nSzp + 1);
		}

		const FeUtil::TiXmlElement* pChildModel = ele.FirstChildElement(ele.FirstChild()->Value());
		while (pChildModel)
		{
			std::string strLocalPath = ToStdString(pChildModel->GetText());
			std::size_t szL = strLocalPath.find("\\");
			std::string strTmp = "";
			std::string filePath = "";
			filePath = strPath;
			if (szL != std::string::npos)
			{
				strTmp.assign(strLocalPath,szL + 1,strLocalPath.length());
			}

			if (!m_bService)
			{
				filePath.append(strTmp);
			}
			else
			{
				filePath = strLocalPath;
			}
			m_tempConfig.tiltList().push_back(filePath);

			pChildModel = pChildModel->NextSiblingElement(ele.FirstChild()->Value());
		}

		return true;
	}

	bool CExTiltReaderWriter::AcceptTypeName( const TiXmlElement& ele, const TiXmlAttribute* attr )
	{
		m_tempConfig.tileType() = osgEarth::as<int>(ToStdString(ele.Attribute("type")), 0.0);

		return true;
	}

}
