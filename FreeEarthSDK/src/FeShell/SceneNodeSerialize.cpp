#include <FeUtils/PathRegistry.h>
#include <FeUtils/StrUtil.h>
#include <FeShell/SceneNodeSerialize.h>

using namespace FeUtil::TinyUtil;

namespace FeShell
{
	const osg::Vec4d DEFAULT_COLOR(1.0, 0.0, 0.0, 1.0); 
	const osg::Vec3d DEFAULT_POSITION(0.0, 0.0, 0.0); 
	const osg::Vec3d DEFAULT_POSTURE(0.0, 0.0, 0.0); 
	const osg::Vec3d DEFAULT_SCALE(0.0, 0.0, 0.0); 
	const FeUtil::CFreeViewPoint DEFAULT_VIEWPOINT();


	CMarkDeserialization::CMarkDeserialization( )
		:FeUtil::TiXmlVisitor()
	{
		
	}

	CMarkDeserialization::CMarkDeserialization( FeUtil::CRenderContext* pContext )
	{
		m_opRenderContext = pContext;
	}

	CMarkDeserialization::~CMarkDeserialization()
	{
		m_markList.clear();
	}


	bool CMarkDeserialization::Execute(FeExNode::CExComposeNode* pRootNode, const std::string& strFilePath)
	{
		if(pRootNode && !strFilePath.empty())
		{
			FeUtil::TiXmlDocument document(strFilePath.c_str());

			if(document.LoadFile(FeUtil::TIXML_ENCODING_UTF8))
			{
				m_markList.push_back(pRootNode);
				if(document.Accept(this))
				{
					return true;
				}
			}
		}
		return NULL;
	}

	bool CMarkDeserialization::VisitEnter( const FeUtil::TiXmlElement& ele , const FeUtil::TiXmlAttribute* attr )
	{
		if(AcceptKey("Folder", ele.Value()))
		{
			EnterFolderMark(ele, attr);
		}
		else if(AcceptKey("PointMark", ele.Value()))
		{
			EnterPointMark(ele, attr);
		}
		else if(AcceptKey("ModelMark", ele.Value()))
		{
			EnterModelMark(ele, attr);
		}
		else if(AcceptKey("LineMark", ele.Value()))
		{
			EnterLineMark(ele, attr);
		}
		else if(AcceptKey("PolygonMark", ele.Value()))
		{
			EnterPolygonMark(ele, attr);
		}
		else if(AcceptKey("MilitaryMarkBillboardMark", ele.Value()))
		{
			EnterMiliMarkBillboardMark(ele, attr);
		}
		else if(AcceptKey("MilitaryMarkOverLayMark", ele.Value()))
		{
			EnterMilitaryMarkOverLayMark(ele, attr);
		}

		return true;
	}

	bool CMarkDeserialization::VisitExit( const FeUtil::TiXmlElement& ele )
	{
		if(AcceptKey("Folder", ele.Value()))
		{
			ExitFolderMark(ele);
		}
		return true;
	}

	void CMarkDeserialization::EnterFolderMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExNode::CExComposeNode> rpComposeNode = new FeExNode::CExComposeNode
				(m_opRenderContext.get(),strTitle,strDesc,DEFAULT_POSITION, DEFAULT_COLOR, viewPoint, bVisible);
			rpComposeNode->SetViewPoint(viewPoint);
			(*m_markList.rbegin())->AddNode(rpComposeNode.get());
			m_markList.push_back(rpComposeNode.get());
		}

	}

	void CMarkDeserialization::EnterModelMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bDefault = osgEarth::as<bool>(ToStdString(ele.Attribute("default")),false);
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		std::string strPath = ToStdString(ele.Attribute("path"));

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d vecPosition = ParsePosition(ele);
		osg::Vec3d vecPosture = ParsePosture(ele);
		osg::Vec3d vecScale = ParseRScale(ele);
		osg::Vec4d vecColor = ParseColor(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExNode::CExModelNode> rModelNode = new FeExNode::CExModelNode(
				m_opRenderContext.get(),strTitle, strDesc,vecPosition,vecPosture,vecScale,vecColor, strPath, viewPoint, bVisible);
			(*m_markList.rbegin())->AddNode(rModelNode.get());
		}
	}

	void CMarkDeserialization::EnterLineMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{		
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		double dWidth = osgEarth::as<double>(ToStdString(ele.Attribute("width")),1.0);

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d vecPosition = ParsePosition(ele);
		osg::Vec4d vecLineColor = ParseColor(ele);
		std::vector<osg::Vec3d> vecVertexs = ParseVertex(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExNode::CExLinesNode> rpLineNode=
				new FeExNode::CExLinesNode(m_opRenderContext.get(), strTitle, strDesc,vecPosition,vecLineColor, dWidth, viewPoint, bVisible);
			rpLineNode->SetVertex(vecVertexs);
			(*m_markList.rbegin())->AddNode(rpLineNode.get());
		}
	}

	void CMarkDeserialization::EnterPolygonMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bDefault = osgEarth::as<bool>(ToStdString(ele.Attribute("default")),false);
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		double dWidth = osgEarth::as<double>(ToStdString(ele.Attribute("width")),1.0);

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d vecPosition = ParsePosition(ele);
		osg::Vec4d vecLineColor = ParseColor(ele);
		osg::Vec4d vecFillColor = ParseFillColor(ele);
		std::vector<osg::Vec3d> vecVertexs = ParseVertex(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExNode::CExPolygonNode> rpPolygonNode = new FeExNode::CExPolygonNode(
				m_opRenderContext.get(), strTitle, strDesc,vecPosition,vecLineColor, vecFillColor, dWidth, viewPoint, bVisible);
			rpPolygonNode->SetVertex(vecVertexs);
			(*m_markList.rbegin())->AddNode(rpPolygonNode.get());

		}
	}

	void CMarkDeserialization::EnterMiliMarkBillboardMark(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr)
	{
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bDefault = osgEarth::as<bool>(ToStdString(ele.Attribute("default")), false);
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")), false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		double dScale = osgEarth::as<double>(ToStdString(ele.Attribute("scale")), 1.0);
		double dAngle = osgEarth::as<double>(ToStdString(ele.Attribute("angle")), 0.0);
		std::string strPath = ToStdString(ele.Attribute("path"));

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d vecPos = ParsePosition(ele);
		osg::Vec3d vecScale = ParseRScale(ele);
		osg::Vec4d vecColor = ParseColor(ele);

		if (!m_markList.empty())
		{
			osg::ref_ptr<FeExNode::CExBillboardImage> rModelNode =new FeExNode::CExBillboardImage(
				m_opRenderContext.get(), strTitle, strDesc, strPath,vecPos,vecScale,vecColor,viewPoint,bVisible);
			(*m_markList.rbegin())->AddNode(rModelNode.get());
		}
	}

	void CMarkDeserialization::EnterMilitaryMarkOverLayMark(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr)
	{
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")), false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		osg::Vec3d vecScale = ParseRScale(ele);
		osg::Vec3d vecPosture = ParsePosture(ele);
		osg::Vec4d vecColor = ParseColor(ele);

		std::string strPath = ToStdString(ele.Attribute("path"));

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d vecPos = ParsePosition(ele);

		if (!m_markList.empty())
		{
			osg::ref_ptr<FeExNode::CExOverLayImage> rModelNode =new FeExNode::CExOverLayImage(
				m_opRenderContext.get(), strTitle, strDesc,strPath,vecPos, vecScale, vecColor, viewPoint,bVisible);
			(*m_markList.rbegin())->AddNode(rModelNode.get());
		}
	}

	void CMarkDeserialization::ExitFolderMark( const FeUtil::TiXmlElement& ele)
	{
		m_markList.pop_back();
	}

	void CMarkDeserialization::EnterPointMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = (ToStdString(ele.Attribute("title")));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);

		if(!m_markList.empty())
		{
            osg::Vec3d pos = ParsePosition(ele);
			osg::ref_ptr<FeExNode::CExPlaceNode> rpPlaceNode = 
                new FeExNode::CExPlaceNode(m_opRenderContext.get(), strTitle, strDesc, pos, DEFAULT_COLOR, viewPoint, bVisible);

			(*m_markList.rbegin())->AddNode(rpPlaceNode.get());
		}
	}

	FeUtil::CFreeViewPoint CMarkDeserialization::ParseViewPoint( const FeUtil::TiXmlElement& element )
	{
		FeUtil::CFreeViewPoint viewPoint;
		const FeUtil::TiXmlElement* pViewPointElement = element.FirstChildElement("viewPoint");
		if(pViewPointElement)
		{
			viewPoint.SetViewPoint(
				osgEarth::as<double>(ToStdString(pViewPointElement->Attribute("lon")),0.0),
				osgEarth::as<double>(ToStdString(pViewPointElement->Attribute("lat")),0.0),
				osgEarth::as<double>(ToStdString(pViewPointElement->Attribute("hei")),0.0),
				osgEarth::as<double>(ToStdString(pViewPointElement->Attribute("pitch")),-90.0),
				osgEarth::as<double>(ToStdString(pViewPointElement->Attribute("heading")),-90.0),
				osgEarth::as<double>(ToStdString(pViewPointElement->Attribute("range")),1000.0),
				osgEarth::as<double>(ToStdString(pViewPointElement->Attribute("time")),2.0));
		}

		return viewPoint;
	}

	osg::Vec3d CMarkDeserialization::ParsePosition( const FeUtil::TiXmlElement& element )
	{
		osg::Vec3d vecPos(0.0,0.0,0.0);
		const FeUtil::TiXmlElement* pRotateElement = element.FirstChildElement("position");
		if(pRotateElement)
		{
			vecPos.x() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("lon")),0.0);
			vecPos.y() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("lat")),0.0);
			vecPos.z() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("hei")),0.0);
		}

		return vecPos;
	}

	osg::Vec3d CMarkDeserialization::ParsePosture( const FeUtil::TiXmlElement& element )
	{
		osg::Vec3d vecRotate(0.0,0.0,0.0);
		const FeUtil::TiXmlElement* pRotateElement = element.FirstChildElement("posture");
		if(pRotateElement)
		{
				vecRotate.x() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("pitching")),1.0);
				vecRotate.y() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("yaw")),1.0);
				vecRotate.z() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("roll")),1.0);

		}

		return vecRotate;
	}

	osg::Vec3d CMarkDeserialization::ParseRScale( const FeUtil::TiXmlElement& element )
	{
		osg::Vec3d vecScale(0.0,0.0,0.0);
		const FeUtil::TiXmlElement* pRotateElement = element.FirstChildElement("scale");
		if(pRotateElement)
		{
			vecScale.x() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("x")),1.0);
			vecScale.y() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("y")),1.0);
			vecScale.z() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("z")),1.0);

		}

		return vecScale;
	}

	osg::Vec4d CMarkDeserialization::ParseColor( const FeUtil::TiXmlElement& ele)
	{
		osg::Vec4d vecColor;
		const FeUtil::TiXmlElement* pColor = ele.FirstChildElement("color");
		if(pColor)
		{
			
			vecColor.x() = osgEarth::as<double>(ToStdString(pColor->Attribute("r")),1.0);
			vecColor.y() = osgEarth::as<double>(ToStdString(pColor->Attribute("g")),1.0);
			vecColor.z() = osgEarth::as<double>(ToStdString(pColor->Attribute("b")),0.0);
			vecColor.w() = osgEarth::as<double>(ToStdString(pColor->Attribute("a")),1.0);
		}
		return vecColor;
	}

	osg::Vec4d CMarkDeserialization::ParseFillColor( const FeUtil::TiXmlElement& ele)
	{
		osg::Vec4d vecFillColor;
		const FeUtil::TiXmlElement* pColor = ele.FirstChildElement("fillColor");
		if(pColor)
		{

			vecFillColor.x() = osgEarth::as<double>(ToStdString(pColor->Attribute("r")),1.0);
			vecFillColor.y() = osgEarth::as<double>(ToStdString(pColor->Attribute("g")),1.0);
			vecFillColor.z() = osgEarth::as<double>(ToStdString(pColor->Attribute("b")),0.0);
			vecFillColor.w() = osgEarth::as<double>(ToStdString(pColor->Attribute("a")),1.0);
		}
		return vecFillColor;
	}

	std::vector<osg::Vec3d> CMarkDeserialization::ParseVertex( const FeUtil::TiXmlElement& ele)
	{
		std::vector<osg::Vec3d> vecVertexs;
		vecVertexs.clear();
		const FeUtil::TiXmlElement* pCenterElement = ele.FirstChildElement("vertex");
		while (pCenterElement)
		{
			osg::Vec3d vecVertex;
			vecVertex.x() = osgEarth::as<double>(ToStdString(pCenterElement->Attribute("lon")),114.0);
			vecVertex.y() = osgEarth::as<double>(ToStdString(pCenterElement->Attribute("lat")),34.0);
			vecVertex.z() = osgEarth::as<double>(ToStdString(pCenterElement->Attribute("hei")),0.0);
			vecVertexs.push_back(vecVertex);
			pCenterElement = pCenterElement->NextSiblingElement();
		}

		return vecVertexs;

	}

	CMarkSerialization::CMarkSerialization()
		:FeExNode::CExternNodeVisitor()
	{

	}

	CMarkSerialization::~CMarkSerialization()
	{
		m_markList.clear();
	}

	bool CMarkSerialization::Execute(FeExNode::CExComposeNode* pExternNode, const std::string& strFilePath)
	{
		if(pExternNode && !strFilePath.empty())
		{

			FeUtil::TiXmlDeclaration declar("1.0", "UTF-8", "");
			FeUtil::TiXmlDocument doc(strFilePath.c_str());

			doc.InsertEndChild(declar);

			FeUtil::TiXmlElement* pRootElement = new FeUtil::TiXmlElement("document");
			doc.LinkEndChild(pRootElement);
			m_markList.push_back(pRootElement);

			FeExNode::VecChildren tempChilren = pExternNode->GetChidlren();
			FeExNode::VecChildren::iterator itr = tempChilren.begin();
			for(itr; itr != tempChilren.end(); ++itr)
			{
				(*itr)->Accept(this);
			}

			return doc.SaveFile();
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExNode::CExComposeNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("Folder");
				
				pElement->SetAttribute("title",  externNode.GetTitle().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);
				(*m_markList.rbegin())->LinkEndChild(pElement);

				m_markList.push_back(pElement);
			}
		}
		
		return true;
	}

	bool CMarkSerialization::VisitEnter( FeExNode::CExPlaceNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("PointMark");
				//pElement->SetAttribute("id",externNode.GetID());
				pElement->SetAttribute("title",  externNode.GetTitle().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

                FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition());
				pElement->LinkEndChild(pMarkPosition);

				(*m_markList.rbegin())->LinkEndChild(pElement);

			}
		}
		return true;
	}

	bool CMarkSerialization::VisitEnter( FeExNode::CExModelNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("ModelMark");
				//pElement->SetAttribute("id",externNode.GetID());
				pElement->SetAttribute("title",  externNode.GetTitle().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetAttribute("path", externNode.GetModelPath().c_str());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pMarkScale = BuildMarkScaleElement(externNode.GetScale());
				pElement->LinkEndChild(pMarkScale);

				FeUtil::TiXmlElement* pMarkRotate = BuildMarkPostureElement(externNode.GetPosture());
				pElement->LinkEndChild(pMarkRotate);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition());
				pElement->LinkEndChild(pMarkPosition);

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}
		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExNode::CExLinesNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("LineMark");
				//pElement->SetAttribute("id",externNode.GetID());
				pElement->SetAttribute("title",  externNode.GetTitle().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

                FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetColor());
				pElement->LinkEndChild(pLineColor);

				std::vector<osg::Vec3d> vecVectexs;
				BuildVertex(pElement,externNode.AsVertexs());

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}
		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExNode::CExPolygonNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("PolygonMark");
				//pElement->SetAttribute("id",externNode.GetID());
				pElement->SetAttribute("title",  externNode.GetTitle().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

                FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetColor());
				pElement->LinkEndChild(pLineColor);

                FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetFillColor());
				pElement->LinkEndChild(pFillColor);

				BuildVertex(pElement,externNode.AsVertexs());

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter(FeExNode::CExBillboardImage& externNode)
	{
		if (externNode.GetID() != MARK_ROOT_ID)
		{
			if (!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("MilitaryMarkBillboardMark");
				//pElement->SetAttribute("id", externNode.GetID());
				pElement->SetAttribute("title", externNode.GetTitle().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				FeUtil::TiXmlElement* pMarkScale = BuildMarkScaleElement(externNode.GetScale());
				pElement->LinkEndChild(pMarkScale);

				pElement->SetAttribute("path", externNode.GetImagePath().c_str());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition());
				pElement->LinkEndChild(pMarkPosition);

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}
		return false;
	}

	bool CMarkSerialization::VisitEnter(FeExNode::CExOverLayImage& externNode)
	{
		if (externNode.GetID() != MARK_ROOT_ID)
		{
			if (!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("MilitaryMarkOverLayMark");
				//pElement->SetAttribute("id", externNode.GetID());
				pElement->SetAttribute("title", externNode.GetTitle().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				FeUtil::TiXmlElement* pMarkScale = BuildMarkScaleElement(externNode.GetScale());
				pElement->LinkEndChild(pMarkScale);

				FeUtil::TiXmlElement* pMarkRotate = BuildMarkPostureElement(externNode.GetPosture());
				pElement->LinkEndChild(pMarkRotate);

				FeUtil::TiXmlElement* pMarkColor = BuildLineColor(externNode.GetColor());
				pElement->LinkEndChild(pMarkColor);

				pElement->SetAttribute("path", externNode.GetImagePath().c_str());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition());
				pElement->LinkEndChild(pMarkPosition);

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}
		return false;
	}

	bool CMarkSerialization::VisitEnter(FeExNode::ExMilitarySignNode& externNode)
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("MilitarySignMark");
				//pElement->SetAttribute("id",externNode.GetID().c_str());
				pElement->SetAttribute("title",  externNode.GetTitle().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetColor());
				pElement->LinkEndChild(pLineColor);

				FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetFillColor());
				pElement->LinkEndChild(pFillColor);

				if(pElement)
				{
					osg::Vec3d llh = externNode.GetStartPosition();
					FeUtil::TiXmlElement * pVertex = new FeUtil::TiXmlElement("vertex");
					pVertex->SetDoubleAttribute("lon", llh.x());
					pVertex->SetDoubleAttribute("lat", llh.y());
					pVertex->SetDoubleAttribute("hei", llh.z());
					pElement->LinkEndChild(pVertex);

					llh = externNode.GetEndPosition();
					pVertex = new FeUtil::TiXmlElement("vertex");
					pVertex->SetDoubleAttribute("lon", llh.x());
					pVertex->SetDoubleAttribute("lat", llh.y());
					pVertex->SetDoubleAttribute("hei", llh.z());
					pElement->LinkEndChild(pVertex);
				}

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	bool CMarkSerialization::VisitExit( FeExNode::CExComposeNode& externNode )
	{
		m_markList.pop_back();
		return true;
	}

	FeUtil::TiXmlElement* CMarkSerialization::BuildViewPointElement( const FeUtil::CFreeViewPoint& viewPoint )
	{
		FeUtil::TiXmlElement * pViewPoint = new FeUtil::TiXmlElement("viewPoint");
		pViewPoint->SetDoubleAttribute("lon", viewPoint.GetLon());
		pViewPoint->SetDoubleAttribute("lat", viewPoint.GetLat());
		pViewPoint->SetDoubleAttribute("hei", viewPoint.GetHei());
		pViewPoint->SetDoubleAttribute("heading", viewPoint.GetHeading());
		pViewPoint->SetDoubleAttribute("pitch", viewPoint.GetPitch());
		pViewPoint->SetDoubleAttribute("range", viewPoint.GetRange());
		pViewPoint->SetDoubleAttribute("time", viewPoint.GetTime());
		return pViewPoint;
	}

	FeUtil::TiXmlElement* CMarkSerialization::BuildMarkPositionElement( const osg::Vec3d& value )
	{
		FeUtil::TiXmlElement * pElement = new FeUtil::TiXmlElement("position");
		pElement->SetDoubleAttribute("lon", value.x());
		pElement->SetDoubleAttribute("lat", value.y());
		pElement->SetDoubleAttribute("hei", value.z());
		return pElement;
	}

	FeUtil::TiXmlElement* CMarkSerialization::BuildMarkPostureElement( const osg::Vec3d& value )
	{
		FeUtil::TiXmlElement * pElement = new FeUtil::TiXmlElement("posture");
		pElement->SetDoubleAttribute("patch", value.x());
		pElement->SetDoubleAttribute("yaw", value.y());
		pElement->SetDoubleAttribute("roll", value.z());
		return pElement;
	}

	FeUtil::TiXmlElement* CMarkSerialization::BuildMarkScaleElement( const osg::Vec3d& value )
	{
		FeUtil::TiXmlElement * pElement = new FeUtil::TiXmlElement("scale");
		pElement->SetDoubleAttribute("x", value.x());
		pElement->SetDoubleAttribute("y", value.y());
		pElement->SetDoubleAttribute("z", value.z());
		return pElement;
	}

	FeUtil::TiXmlElement* CMarkSerialization::BuildLineColor(const osg::Vec4d& veccolor)
	{
		FeUtil::TiXmlElement * pColor = new FeUtil::TiXmlElement("color");
		pColor->SetDoubleAttribute("r", veccolor.x());
		pColor->SetDoubleAttribute("g", veccolor.y());
		pColor->SetDoubleAttribute("b", veccolor.z());
		pColor->SetDoubleAttribute("a", veccolor.a());
		return pColor;
	}

	FeUtil::TiXmlElement* CMarkSerialization::BuildFillColor(const osg::Vec4d& veccolor)
	{
		FeUtil::TiXmlElement * pColor = new FeUtil::TiXmlElement("fillColor");
		pColor->SetDoubleAttribute("r", veccolor.x());
		pColor->SetDoubleAttribute("g", veccolor.y());
		pColor->SetDoubleAttribute("b", veccolor.z());
		pColor->SetDoubleAttribute("a", veccolor.a());
		return pColor;
	}

	void CMarkSerialization::BuildVertex(FeUtil::TiXmlElement* pParentElement,std::vector<osg::Vec3d> vecVertexs)
	{
		if(!pParentElement) return;

		for(std::vector<osg::Vec3d>::iterator itr = vecVertexs.begin(); itr != vecVertexs.end(); ++itr)
		{
			FeUtil::TiXmlElement * pVertex = new FeUtil::TiXmlElement("vertex");
			pVertex->SetDoubleAttribute("lon", (*itr).x());
			pVertex->SetDoubleAttribute("lat", (*itr).y());
			pVertex->SetDoubleAttribute("hei", (*itr).z());
			pParentElement->LinkEndChild(pVertex);
		}
	}



}
