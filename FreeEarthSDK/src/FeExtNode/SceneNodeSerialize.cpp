#include <FeUtils/PathRegistry.h>
#include <FeUtils/StrUtil.h>
#include <FeExtNode/SceneNodeSerialize.h>
#include <FeExtNode/ExComposeNode.h>
#include <FeExtNode/FreeMarkSys.h>

using namespace FeUtil::TinyUtil;

namespace FeExtNode
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

	CMarkDeserialization::CMarkDeserialization( FeUtil::CRenderContext* pContext , CFreeMarkSys* pMarkSys)
	{
		m_opRenderContext = pContext;
		m_opMarkSys = pMarkSys;
	}

	CMarkDeserialization::~CMarkDeserialization()
	{
		m_markList.clear();
	}


	bool CMarkDeserialization::Execute(FeExtNode::CExComposeNode* pRootNode, const std::string& strFilePath)
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
		return false;
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
		else if(AcceptKey("LodModelMark", ele.Value()))
		{
			EnterLodModelMark(ele, attr);
		}
		else if(AcceptKey("LineMark", ele.Value()))
		{
			EnterLineMark(ele, attr);
		}
		else if(AcceptKey("PolygonMark", ele.Value()))
		{
			EnterPolygonMark(ele, attr);
		}
		else if(AcceptKey("BillboardMark", ele.Value()))
		{
			EnterBillboardMark(ele, attr);
		}
		else if(AcceptKey("OverLayMark", ele.Value()))
		{
			EnterOverLayMark(ele, attr);
		}
		else if(AcceptKey("SectorMark", ele.Value()))
		{
			EnterSectorMark(ele, attr);
		}
		else if(AcceptKey("ArcMark", ele.Value()))
		{
			EnterArcMark(ele, attr);
		}
		else if(AcceptKey("CircleMark", ele.Value()))
		{
			EnterCircleMark(ele, attr);
		}
		else if(AcceptKey("EllipseMark", ele.Value()))
		{
			EnterEllipseMark(ele, attr);
		}
		else if(AcceptKey("TextMark", ele.Value()))
		{
			EnterTextMark(ele, attr);
		}
		else if(AcceptKey("RectMark", ele.Value()))
		{
			EnterRectMark(ele, attr);
		}
		else if(AcceptKey("TileModelMark", ele.Value()))
		{
			EnterTiltModelMark(ele, attr);
		}
		else if(AcceptKey("StraightArrowMark", ele.Value()))
		{
			EnterStraightArrowMark(ele, attr);
		}
		else if(AcceptKey("AssaultMark", ele.Value()))
		{
			EnterAssaultMark(ele, attr);
		}
		else if(AcceptKey("AttackMark", ele.Value()))
		{
			EnterAttackMark(ele, attr);
		}
		else if(AcceptKey("LabelMark", ele.Value()))
		{
			EnterLabelMark(ele, attr);
		}
		else if(AcceptKey("ParticleMark", ele.Value()))
		{
			EnterParticleMark(ele, attr);
		}
		else if(AcceptKey("LabelItem", ele.Value()))
		{
			EnterLabelItem(ele, attr);
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
			osg::ref_ptr<FeExtNode::CExComposeNodeOption> optNode = new FeExtNode::CExComposeNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			osg::ref_ptr<FeExtNode::CExComposeNode> rpNode = new FeExtNode::CExComposeNode(optNode);
			(*m_markList.rbegin())->InsertNode(rpNode.get());
			m_markList.push_back(rpNode.get());
		}

	}

	void CMarkDeserialization::EnterLodModelMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bDefault = osgEarth::as<bool>(ToStdString(ele.Attribute("default")),false);
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d vecPosition = ParsePosition(ele);
		osg::Vec3d vecPosture = ParsePosture(ele);
		osg::Vec3d vecScale = ParseRScale(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExLodModelNodeOption> optNode = new FeExtNode::CExLodModelNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			osg::ref_ptr<FeExtNode::CExLodModelNode> rModelNode = new FeExtNode::CExLodModelNode(optNode, m_opRenderContext.get());
			rModelNode->SetPositionByLLH(vecPosition);
			rModelNode->SetScale(vecScale);
			rModelNode->SetRotate(vecPosture);

			/// 解析子节点
			std::string strModelPath;
			double dVisibelMin, dVisibelMax;
			
			ParseLodModel(ele, "bestModel", strModelPath, dVisibelMin, dVisibelMax);
			rModelNode->SetLodModelNode(CExLodModelNode::LOD_MODEL_BEST, strModelPath, dVisibelMin, dVisibelMax);

			ParseLodModel(ele, "normalModel", strModelPath, dVisibelMin, dVisibelMax);
			rModelNode->SetLodModelNode(CExLodModelNode::LOD_MODEL_NORMAL, strModelPath, dVisibelMin, dVisibelMax);

			ParseLodModel(ele, "simpleModel", strModelPath, dVisibelMin, dVisibelMax);
			rModelNode->SetLodModelNode(CExLodModelNode::LOD_MODEL_SIMPLE, strModelPath, dVisibelMin, dVisibelMax);

			(*m_markList.rbegin())->InsertNode(rModelNode.get());
		}
	}

	void CMarkDeserialization::EnterTiltModelMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));
		std::string strPath = ToStdString(ele.Attribute("path"));

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d vecPosition = ParsePosition(ele);
		osg::Vec3d vecPosture = ParsePosture(ele);
		osg::Vec3d vecScale = ParseRScale(ele);
		osg::Vec3d vecOffset = ParseOffset(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExTiltModelNodeOption> optNode = new FeExtNode::CExTiltModelNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->tiltPath() = strPath;
			osg::ref_ptr<FeExtNode::CExTiltModelNode> rModelNode = new FeExtNode::CExTiltModelNode(m_opRenderContext.get(), optNode);
			rModelNode->SetPosition(vecPosition);
			rModelNode->SetScale(vecScale);
			rModelNode->SetRotate(vecPosture);
			rModelNode->SetOffset(vecOffset);

			(*m_markList.rbegin())->InsertNode(rModelNode.get());
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
			osg::ref_ptr<FeExtNode::CExLineNodeOption> optNode = new FeExtNode::CExLineNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->lineColor() = vecLineColor;
			osg::ref_ptr<FeExtNode::CExLineNode> rpLineNode= new FeExtNode::CExLineNode(m_opRenderContext.get(), optNode);
			osg::ref_ptr<osg::Vec3dArray> vertexsArray = new osg::Vec3dArray;

            for(int i = 0; i < vecVertexs.size(); i++)
            {
                vertexsArray->push_back(vecVertexs.at(i));
            }

			rpLineNode->SetVertex(vertexsArray);
			(*m_markList.rbegin())->InsertNode(rpLineNode.get());
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
			osg::ref_ptr<FeExtNode::CExPolygonNodeOption> optNode = new FeExtNode::CExPolygonNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->lineColor() = vecLineColor;
			optNode->fillColor() = vecFillColor;
			osg::ref_ptr<FeExtNode::CExPolygonNode> rpPolygonNode = new FeExtNode::CExPolygonNode(m_opRenderContext.get(), optNode);
			osg::ref_ptr<osg::Vec3dArray> vertexsArray = new osg::Vec3dArray;

            for(int i = 0; i < vecVertexs.size(); i++)
            {
                vertexsArray->push_back(vecVertexs.at(i));
            }

			rpPolygonNode->SetVertex(vertexsArray);
			(*m_markList.rbegin())->InsertNode(rpPolygonNode.get());
		}
	}

	void CMarkDeserialization::EnterBillboardMark(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr)
	{
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bDefault = osgEarth::as<bool>(ToStdString(ele.Attribute("default")), false);
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")), false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		double dScale = osgEarth::as<double>(ToStdString(ele.Attribute("scale")), 1.0);
		double dAngle = osgEarth::as<double>(ToStdString(ele.Attribute("angle")), 0.0);
		std::string strPath = ToStdString(ele.Attribute("path"));
		osg::ref_ptr<osg::Image> image = osgDB::readImageFile(strPath);
		if(!image.valid())
		{
			strPath = FeFileReg->GetFullPath("texture/mark/mark_notfind.png");
		}

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d vecPos = ParsePosition(ele);
		osg::Vec3d vecScale = ParseRScale(ele);
		osg::Vec4d vecColor = ParseColor(ele);

		if (!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExBillBoardNodeOption> optNode = new FeExtNode::CExBillBoardNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->imagePath() = strPath;
			osg::ref_ptr<FeExtNode::CExBillBoardNode> rModelNode =new FeExtNode::CExBillBoardNode(m_opRenderContext.get(), optNode);
			rModelNode->SetPosition(osgEarth::GeoPoint(m_opRenderContext->GetMapSRS(), vecPos));
			rModelNode->SetImageSize(osg::Vec2(vecScale.x(), vecScale.y()));
			(*m_markList.rbegin())->InsertNode(rModelNode.get());
		}
	}

	void CMarkDeserialization::EnterOverLayMark(const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr)
	{
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")), false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		osg::Vec3d vecScale = ParseRScale(ele);
		osg::Vec3d vecPosture = ParsePosture(ele);
		osg::Vec4d vecColor = ParseColor(ele);

		std::string strPath = ToStdString(ele.Attribute("path"));
		osg::ref_ptr<osg::Image> image = osgDB::readImageFile(strPath);
		if(!image.valid())
		{
			strPath = FeFileReg->GetFullPath("texture/mark/mark_notfind.png");
		}
		
		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d vecPos = ParsePosition(ele);

		if (!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExOverLayNodeOption> optNode = new FeExtNode::CExOverLayNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->imagePath() = strPath;
			optNode->imageSize() = osg::Vec2(vecScale.x(), vecScale.y());
			optNode->geoPoint() = osgEarth::GeoPoint(m_opRenderContext->GetMapSRS(), vecPos);
			osg::ref_ptr<FeExtNode::CExOverLayNode> rModelNode =new FeExtNode::CExOverLayNode(m_opRenderContext.get(), optNode);
			
			(*m_markList.rbegin())->InsertNode(rModelNode.get());
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
		std::string strPath = ToStdString(ele.Attribute("path"));
		osg::ref_ptr<osg::Image> image = osgDB::readImageFile(strPath);
		if(!image.valid())
		{
			strPath = FeFileReg->GetFullPath("texture/mark/mark_notfind.png");
		}

		double dSize = osgEarth::as<double>(ToStdString(ele.Attribute("size")),1.0);
		osg::Vec4d vecLineColor = ParseColor(ele);

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);

		if(!m_markList.empty())
		{
            osg::Vec3d pos = ParsePosition(ele);

			osg::ref_ptr<FeExtNode::CExPlaceNodeOption> optNode = new FeExtNode::CExPlaceNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->imagePath() = strPath;
			optNode->TextColor() = vecLineColor;
			optNode->TextSize() = dSize;
			optNode->geoPoint() = osgEarth::GeoPoint(m_opRenderContext->GetMapSRS(), pos);
			osg::ref_ptr<FeExtNode::CExPlaceNode> rpPlaceNode = new FeExtNode::CExPlaceNode(m_opRenderContext.get(), optNode);

			(*m_markList.rbegin())->InsertNode(rpPlaceNode.get());
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
				osgEarth::as<double>(ToStdString(pViewPointElement->Attribute("heading")),-90.0),
				osgEarth::as<double>(ToStdString(pViewPointElement->Attribute("pitch")),-90.0),
				osgEarth::as<double>(ToStdString(pViewPointElement->Attribute("range")),1000.0),
				osgEarth::as<double>(ToStdString(pViewPointElement->Attribute("time")),2.0));
		}

		return viewPoint;
	}


	osg::Vec3d CMarkDeserialization::ParseOffset( const FeUtil::TiXmlElement& element )
	{
		osg::Vec3d vecPos(0.0,0.0,0.0);
		const FeUtil::TiXmlElement* pRotateElement = element.FirstChildElement("offset");
		if(pRotateElement)
		{
			vecPos.x() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("x")),0.0);
			vecPos.y() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("y")),0.0);
			vecPos.z() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("z")),0.0);
		}

		return vecPos;
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

	osg::Vec3d CMarkDeserialization::ParseAngle(const FeUtil::TiXmlElement& element)
	{
		osg::Vec3d vecPos(0.0,0.0,0.0);
		const FeUtil::TiXmlElement* pRotateElement = element.FirstChildElement("angle");
		if(pRotateElement)
		{
			vecPos.x() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("x")),0.0);
			vecPos.y() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("y")),0.0);
			vecPos.z() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("z")),0.0);
		}

		return vecPos;
	}

	osg::Vec3d CMarkDeserialization::ParsePosture( const FeUtil::TiXmlElement& element )
	{
		osg::Vec3d vecRotate(0.0,0.0,0.0);
		const FeUtil::TiXmlElement* pRotateElement = element.FirstChildElement("posture");
		if(pRotateElement)
		{
			vecRotate.x() = osgEarth::as<double>(ToStdString(pRotateElement->Attribute("patch")),1.0);
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

	void CMarkDeserialization::EnterSectorMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = (ToStdString(ele.Attribute("title")));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));
		
		osg::Vec4d vecLineColor = ParseColor(ele);
		osg::Vec4d vecFillColor = ParseFillColor(ele);
		double dRadius = osgEarth::as<double>(ToStdString(ele.Attribute("radius")),1.0); 
		double dArcStart = osgEarth::as<double>(ToStdString(ele.Attribute("arcStart")),1.0); 
		double dArcEnd = osgEarth::as<double>(ToStdString(ele.Attribute("arcEnd")),1.0); 

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d pos = ParsePosition(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExSectorNodeOption> optNode = new FeExtNode::CExSectorNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->lineColor() = vecLineColor;
			optNode->fillColor() = vecFillColor;
			optNode->Radius() = dRadius;
			optNode->ArcStart() = dArcStart;
			optNode->ArcEnd() = dArcEnd;
			optNode->geoPoint() = osgEarth::GeoPoint(m_opRenderContext->GetMapSRS(), pos);
			osg::ref_ptr<FeExtNode::CExSectorNode> rpPlaceNode = new FeExtNode::CExSectorNode(m_opRenderContext.get(), optNode);

			(*m_markList.rbegin())->InsertNode(rpPlaceNode.get());
		}
	}

	void CMarkDeserialization::EnterArcMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = (ToStdString(ele.Attribute("title")));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		osg::Vec4d vecLineColor = ParseColor(ele);
		osg::Vec4d vecFillColor = ParseFillColor(ele);
		double dRadius = osgEarth::as<double>(ToStdString(ele.Attribute("radius")),1.0); 
		double dArcStart = osgEarth::as<double>(ToStdString(ele.Attribute("arcStart")),1.0); 
		double dArcEnd = osgEarth::as<double>(ToStdString(ele.Attribute("arcEnd")),1.0); 

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d pos = ParsePosition(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExArcNodeOption> optNode = new FeExtNode::CExArcNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->lineColor() = vecLineColor;
			optNode->fillColor() = vecFillColor;
			optNode->Radius() = dRadius;
			optNode->ArcStart() = dArcStart;
			optNode->ArcEnd() = dArcEnd;
			optNode->geoPoint() = osgEarth::GeoPoint(m_opRenderContext->GetMapSRS(), pos);
			osg::ref_ptr<FeExtNode::CExArcNode> rpPlaceNode = new FeExtNode::CExArcNode(m_opRenderContext.get(), optNode);

			(*m_markList.rbegin())->InsertNode(rpPlaceNode.get());
		}
	}

	void CMarkDeserialization::EnterCircleMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = (ToStdString(ele.Attribute("title")));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		osg::Vec4d vecLineColor = ParseColor(ele);
		osg::Vec4d vecFillColor = ParseFillColor(ele);
		double dRadius = osgEarth::as<double>(ToStdString(ele.Attribute("radius")),1.0); 

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d pos = ParsePosition(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExCircleNodeOption> optNode = new FeExtNode::CExCircleNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->lineColor() = vecLineColor;
			optNode->fillColor() = vecFillColor;
			optNode->Radius() = dRadius;
			optNode->geoPoint() = osgEarth::GeoPoint(m_opRenderContext->GetMapSRS(), pos);
			osg::ref_ptr<FeExtNode::CExCircleNode> rpPlaceNode = new FeExtNode::CExCircleNode(m_opRenderContext.get(), optNode);

			(*m_markList.rbegin())->InsertNode(rpPlaceNode.get());
		}
	}

	void CMarkDeserialization::EnterEllipseMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = (ToStdString(ele.Attribute("title")));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		osg::Vec4d vecLineColor = ParseColor(ele);
		osg::Vec4d vecFillColor = ParseFillColor(ele);
		double dsRadius = osgEarth::as<double>(ToStdString(ele.Attribute("sRadius")),1.0); 
		double dlRadius = osgEarth::as<double>(ToStdString(ele.Attribute("lRadius")),1.0); 

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d pos = ParsePosition(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExEllipseNodeOption> optNode = new FeExtNode::CExEllipseNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->lineColor() = vecLineColor;
			optNode->fillColor() = vecFillColor;
			optNode->ShortRadiu() = dsRadius;
			optNode->LongRadiu() = dlRadius;
			optNode->geoPoint() = osgEarth::GeoPoint(m_opRenderContext->GetMapSRS(), pos);
			osg::ref_ptr<FeExtNode::CExEllipseNode> rpPlaceNode = new FeExtNode::CExEllipseNode(m_opRenderContext.get(), optNode);

			(*m_markList.rbegin())->InsertNode(rpPlaceNode.get());
		}
	}

	void CMarkDeserialization::EnterTextMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = (ToStdString(ele.Attribute("title")));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		double dSize = osgEarth::as<double>(ToStdString(ele.Attribute("size")),1.0);
		osg::Vec4d vecLineColor = ParseColor(ele);

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d pos = ParsePosition(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExTextNodeOption> optNode = new FeExtNode::CExTextNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->geoPoint() = osgEarth::GeoPoint(m_opRenderContext->GetMapSRS(), pos);
			osg::ref_ptr<FeExtNode::CExTextNode> rpPlaceNode = new FeExtNode::CExTextNode(m_opRenderContext.get(), optNode);
			rpPlaceNode->SetTextSize(dSize);
			rpPlaceNode->SetTextColor(vecLineColor);

			(*m_markList.rbegin())->InsertNode(rpPlaceNode.get());
		}
	}

	void CMarkDeserialization::EnterRectMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		double dWidth = osgEarth::as<double>(ToStdString(ele.Attribute("line_width")),1.0);
		double dRectWidth = osgEarth::as<double>(ToStdString(ele.Attribute("rect_width")),1.0);
		double dRectHeight = osgEarth::as<double>(ToStdString(ele.Attribute("rect_height")),1.0);

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d vecPosition = ParsePosition(ele);
		osg::Vec4d vecLineColor = ParseColor(ele);
		osg::Vec4d vecFillColor = ParseFillColor(ele);
		
		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExRectNodeOption> optNode = new FeExtNode::CExRectNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->lineColor() = vecLineColor;
			optNode->fillColor() = vecFillColor;
			optNode->geoPoint() = osgEarth::GeoPoint(m_opRenderContext->GetMapSRS(), vecPosition);
			optNode->lineWidth() = dWidth;
			optNode->Width() = dRectWidth;
			optNode->Height() = dRectHeight;
			osg::ref_ptr<FeExtNode::CExRectNode> rpPolygonNode = new FeExtNode::CExRectNode(m_opRenderContext.get(), optNode);
			
			(*m_markList.rbegin())->InsertNode(rpPolygonNode.get());
		}
	}

	void CMarkDeserialization::ParseLodModel( const FeUtil::TiXmlElement& ele, 
		const std::string& eleName, std::string& path, double& min, double& max )
	{
		const FeUtil::TiXmlElement* pElement = ele.FirstChildElement(eleName.c_str());
		if(pElement)
		{
			path = ToStdString(pElement->Attribute("path"));
			min = osgEarth::as<double>(ToStdString(pElement->Attribute("min")),1.0);
			max = osgEarth::as<double>(ToStdString(pElement->Attribute("max")),1.0);
		}
		else
		{
			path = "";
			min = max = 0;
		}
	}

	void CMarkDeserialization::EnterStraightArrowMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
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
			osg::ref_ptr<FeExtNode::CExStraightArrowNodeOption> optNode = new FeExtNode::CExStraightArrowNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->lineColor() = vecLineColor;
			optNode->fillColor() = vecFillColor;
			optNode->lineWidth() = dWidth;
			osg::ref_ptr<FeExtNode::CExStraightArrowNode> rpNode = new FeExtNode::CExStraightArrowNode(m_opRenderContext.get(), optNode);
			osg::ref_ptr<osg::Vec3dArray> vertexsArray = new osg::Vec3dArray;

			for(int i = 0; i < vecVertexs.size(); i++)
			{
				vertexsArray->push_back(vecVertexs.at(i));
			}

			rpNode->SetVertex(vertexsArray);
			(*m_markList.rbegin())->InsertNode(rpNode.get());
		}
	}

	void CMarkDeserialization::EnterAssaultMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
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
			osg::ref_ptr<FeExtNode::CExAssaultNodeOption> optNode = new FeExtNode::CExAssaultNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->lineColor() = vecLineColor;
			optNode->fillColor() = vecFillColor;
			optNode->lineWidth() = dWidth;
			osg::ref_ptr<FeExtNode::CExAssaultNode> rpNode = new FeExtNode::CExAssaultNode(m_opRenderContext.get(), optNode);
			osg::ref_ptr<osg::Vec3dArray> vertexsArray = new osg::Vec3dArray;

			for(int i = 0; i < vecVertexs.size(); i++)
			{
				vertexsArray->push_back(vecVertexs.at(i));
			}

			rpNode->SetVertex(vertexsArray);
			(*m_markList.rbegin())->InsertNode(rpNode.get());
		}
	}

	void CMarkDeserialization::EnterAttackMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
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
			osg::ref_ptr<FeExtNode::CExAttackNodeOption> optNode = new FeExtNode::CExAttackNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->lineColor() = vecLineColor;
			optNode->fillColor() = vecFillColor;
			optNode->lineWidth() = dWidth;
			osg::ref_ptr<FeExtNode::CExAttackNode> rpNode = new FeExtNode::CExAttackNode(m_opRenderContext.get(), optNode);
			osg::ref_ptr<osg::Vec3dArray> vertexsArray = new osg::Vec3dArray;

			for(int i = 0; i < vecVertexs.size(); i++)
			{
				vertexsArray->push_back(vecVertexs.at(i));
			}

			rpNode->SetVertex(vertexsArray);
			(*m_markList.rbegin())->InsertNode(rpNode.get());
		}
	}

	void CMarkDeserialization::EnterParticleMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		std::string strTitle = ToStdString(ele.Attribute("title"));
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d vecPosition = ParsePosition(ele);
		osg::Vec3d vecAngle = ParseAngle(ele);
		double dScale = osgEarth::as<double>(ToStdString(ele.Attribute("scale")), 1.0);
		std::string strPath = ToStdString(ele.Attribute("path"));

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExParticleNodeOption> optNode = new FeExtNode::CExParticleNodeOption;
			optNode->name() = strTitle;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->GetPath() = strPath;
			optNode->GetPosition() = vecPosition;
			optNode->GetAngle() = vecAngle;
			optNode->GetScale() = dScale;
			osg::ref_ptr<FeExtNode::CExParticleNode> rpNode = new FeExtNode::CExParticleNode(optNode, m_opRenderContext.get());
			rpNode->SetParticlePath(strPath);
			rpNode->SetParticlePosition(vecPosition);
			rpNode->SetParticleAngle(vecAngle);

			(*m_markList.rbegin())->InsertNode(rpNode.get());
		}
	}

	void CMarkDeserialization::EnterLabelMark( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		std::string strDesc = ToStdString(ele.Attribute("description"));

		FeUtil::CFreeViewPoint viewPoint = ParseViewPoint(ele);
		osg::Vec3d pos = ParsePosition(ele);

		/// ID
		m_rpLabelNodeParent.first = osgEarth::as<int>(ToStdString(ele.Attribute("ID")), 1);

		/// 宽高
		double width = osgEarth::as<double>(ToStdString(ele.Attribute("width")), 1.0);
		double height = osgEarth::as<double>(ToStdString(ele.Attribute("height")), 1.0);
		
		/// 背景图图片路径
		std::string strBkImagePath = ToStdString(ele.Attribute("imagePath"));
		
		/// 文本字符串
		std::string strText = (ToStdString(ele.Attribute("textString")));
		std::string strTitle = (ToStdString(ele.Attribute("title")));
		
		/// 文本对齐方式
		int nTextAlign = osgEarth::as<int>(ToStdString(ele.Attribute("textAlign")), 0);
		
		/// 字体名称
		std::string strFontName = (ToStdString(ele.Attribute("fontName")));
		
		/// 字体大小
		double fontSize = osgEarth::as<double>(ToStdString(ele.Attribute("fontSize")), 1.0);
		
		/// 文本颜色
		osg::Vec4d vecTextColor = ParseColor(ele);
		
		/// 背景色
		osg::Vec4d vecBkColor = ParseFillColor(ele);

		if(!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExLabelNodeOption> optNode = new FeExtNode::CExLabelNodeOption;
			optNode->description() = strDesc;
			optNode->visible() = bVisible;
			optNode->viewPoint() = viewPoint;
			optNode->BindingLLH() = pos;
			optNode->Width() = width;
			optNode->Height() = height;
			optNode->BackgroundImagePath() = strBkImagePath;
			optNode->Text() = strText;
			optNode->name() = strTitle;
			optNode->Alignment() = (FeExtNode::CExLabelNodeOption::E_TEXT_ALIGNMENT)nTextAlign;
			optNode->FontName() = strFontName;
			optNode->FontSize() = fontSize;
			optNode->TextColor() = vecTextColor;
			optNode->BackgroundColor() = vecBkColor;
			
			m_rpLabelNodeParent.second = new FeExtNode::CExLabelNode(m_opRenderContext.get(), optNode);
			m_rpLabelNodeParent.second->SetClampToTerrain(true);
			//(*m_markList.rbegin())->InsertNode(m_rpLabelNodeParent.second.get());

			if(m_opMarkSys.valid()) m_opMarkSys->AddMark((*m_markList.rbegin()), m_rpLabelNodeParent.second.get());
		}
	}

	void CMarkDeserialization::EnterLabelItem( const FeUtil::TiXmlElement& ele, const FeUtil::TiXmlAttribute* attr )
	{
		bool bVisible = osgEarth::as<bool>(ToStdString(ele.Attribute("visible")),false);
		
		osg::ref_ptr<FeExtNode::CExLabelNode> rpLabelParent = 0;
		osg::ref_ptr<FeExtNode::CExLabelItemNode> rpLabelItemParent = 0;

		/// ID
		int nParentID = osgEarth::as<int>(ToStdString(ele.Attribute("ParentID")), 1);

		/// 找到父节点
		if(nParentID == m_rpLabelNodeParent.first)
		{
			rpLabelParent = m_rpLabelNodeParent.second;
		}
		else 
		{
			while(!m_stackLabel.empty())
			{
				if(m_stackLabel.top().first == nParentID)
				{
					rpLabelItemParent = m_stackLabel.top().second;
					break;
				}
				else
				{
					m_stackLabel.pop();
				}
			}
		}

		std::pair< int ,osg::ref_ptr<FeExtNode::CExLabelItemNode> > curItem;
		curItem.first = osgEarth::as<int>(ToStdString(ele.Attribute("ID")), 1);
		
		/// 宽高
		double width = osgEarth::as<double>(ToStdString(ele.Attribute("width")), 1.0);
		double height = osgEarth::as<double>(ToStdString(ele.Attribute("height")), 1.0);

		/// 背景图图片路径
		std::string strBkImagePath = ToStdString(ele.Attribute("imagePath"));

		/// 文本字符串
		std::string strText = (ToStdString(ele.Attribute("textString")));
		std::string strTitle = (ToStdString(ele.Attribute("title")));

		/// 文本对齐方式
		int nTextAlign = osgEarth::as<int>(ToStdString(ele.Attribute("textAlign")), 0);

		/// 字体名称
		std::string strFontName = (ToStdString(ele.Attribute("fontName")));

		/// 字体大小
		double fontSize = osgEarth::as<double>(ToStdString(ele.Attribute("fontSize")), 1.0);

		/// 文本颜色
		osg::Vec4d vecTextColor = ParseColor(ele);

		/// 背景色
		osg::Vec4d vecBkColor = ParseFillColor(ele);

		//if(!m_markList.empty())
		{
			osg::ref_ptr<FeExtNode::CExLabelItemOption> optNode = new FeExtNode::CExLabelItemOption;
			optNode->visible() = bVisible;
			optNode->Width() = width;
			optNode->Height() = height;
			optNode->BackgroundImagePath() = strBkImagePath;
			optNode->Text() = strText;
			optNode->name() = strTitle;
			optNode->Alignment() = (FeExtNode::CExLabelNodeOption::E_TEXT_ALIGNMENT)nTextAlign;
			optNode->FontName() = strFontName;
			optNode->FontSize() = fontSize;
			optNode->TextColor() = vecTextColor;
			optNode->BackgroundColor() = vecBkColor;

			curItem.second = new FeExtNode::CExLabelItemNode(m_opRenderContext.get(), optNode);
			m_stackLabel.push(curItem);

			if(rpLabelItemParent.valid())
			{
				rpLabelItemParent->AddChildItem(curItem.second.get());
			}
			else if(rpLabelParent.valid())
			{
				rpLabelParent->AddLabelItem(curItem.second.get());
			}
		}
	}

}



namespace FeExtNode
{
	typedef std::vector<CExternNode> VecChildren;

	CMarkSerialization::CMarkSerialization()
		:FeExtNode::CExternNodeVisitor()
	{

	}

	CMarkSerialization::~CMarkSerialization()
	{
		m_markList.clear();
	}

	bool CMarkSerialization::Execute(FeExtNode::CExComposeNode* pExternNode, const std::string& strFilePath)
	{
		if(pExternNode && !strFilePath.empty())
		{

			FeUtil::TiXmlDeclaration declar("1.0", "UTF-8", "");
			FeUtil::TiXmlDocument doc(strFilePath.c_str());

			doc.InsertEndChild(declar);

			FeUtil::TiXmlElement* pRootElement = new FeUtil::TiXmlElement("document");
			doc.LinkEndChild(pRootElement);
			m_markList.push_back(pRootElement);

			CExComposeNodeOption::VecExternNodes tempChilren = pExternNode->GetChildren();
			CExComposeNodeOption::VecExternNodes::iterator itr = tempChilren.begin();
			for(itr; itr != tempChilren.end(); ++itr)
			{
				(*itr)->Accept(*this);
			}

			return doc.SaveFile();
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExtNode::CExComposeNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("Folder");
				
				pElement->SetAttribute("title",  externNode.GetName().c_str());
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

	bool CMarkSerialization::VisitEnter( FeExtNode::CExPlaceNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("PointMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());
				pElement->SetAttribute("path", externNode.GetImagePath().c_str());
				pElement->SetDoubleAttribute("size", externNode.GetTextSize());

				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetTextColor());
				pElement->LinkEndChild(pLineColor);

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

                FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition().vec3d());
				pElement->LinkEndChild(pMarkPosition);

				(*m_markList.rbegin())->LinkEndChild(pElement);

			}
		}
		return true;
	}

	bool CMarkSerialization::VisitEnter( FeExtNode::CExLodModelNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("LodModelMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pMarkScale = BuildMarkScaleElement(externNode.GetScale());
				pElement->LinkEndChild(pMarkScale);

				FeUtil::TiXmlElement* pMarkRotate = BuildMarkPostureElement(externNode.GetRotate());
				pElement->LinkEndChild(pMarkRotate);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPositionLLH());
				pElement->LinkEndChild(pMarkPosition);

				FeExtNode::CExLodModelNode::LodNodeInfo nodeInfo = externNode.GetLodModelNodeInfo(CExLodModelNode::LOD_MODEL_BEST);
				FeUtil::TiXmlElement* pElementNode = BuildLodNodeElement(nodeInfo, "bestModel");
				if(pElementNode)
				{
					pElement->LinkEndChild(pElementNode);
				}

				nodeInfo = externNode.GetLodModelNodeInfo(CExLodModelNode::LOD_MODEL_NORMAL);
				pElementNode = BuildLodNodeElement(nodeInfo, "normalModel");
				if(pElementNode)
				{
					pElement->LinkEndChild(pElementNode);
				}

				nodeInfo = externNode.GetLodModelNodeInfo(CExLodModelNode::LOD_MODEL_SIMPLE);
				pElementNode = BuildLodNodeElement(nodeInfo, "simpleModel");
				if(pElementNode)
				{
					pElement->LinkEndChild(pElementNode);
				}

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}
		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExtNode::CExTiltModelNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("TileModelMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());
				pElement->SetAttribute("path", externNode.GetTiltPath().c_str());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pMarkScale = BuildMarkScaleElement(externNode.GetScale());
				pElement->LinkEndChild(pMarkScale);

				FeUtil::TiXmlElement* pMarkRotate = BuildMarkPostureElement(externNode.GetRotate());
				pElement->LinkEndChild(pMarkRotate);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition());
				pElement->LinkEndChild(pMarkPosition);

				FeUtil::TiXmlElement* pMarkOffset = BuildMarkOffsetElement(externNode.GetOffset());
				pElement->LinkEndChild(pMarkOffset);

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}
		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExtNode::CExLineNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("LineMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

                FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetLineColor());
				pElement->LinkEndChild(pLineColor);

				std::vector<osg::Vec3d> vecVectexs;
				osg::ref_ptr<osg::Vec3dArray> vertexsArray = new osg::Vec3dArray;
				externNode.GetVertex(vertexsArray);
				BuildVertex(pElement, vertexsArray->asVector());

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}
		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExtNode::CExPolygonNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("PolygonMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

                FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetLineColor());
				pElement->LinkEndChild(pLineColor);

                FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetFillColor());
				pElement->LinkEndChild(pFillColor);

				osg::ref_ptr<osg::Vec3dArray> vertexsArray = new osg::Vec3dArray;
				externNode.GetVertex(vertexsArray);
				BuildVertex(pElement, vertexsArray->asVector());
				
				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter(FeExtNode::CExBillBoardNode& externNode)
	{
		if (externNode.GetID() != MARK_ROOT_ID)
		{
			if (!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("BillboardMark");
				//pElement->SetAttribute("id", externNode.GetID());
				pElement->SetAttribute("title", externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				FeUtil::TiXmlElement* pMarkScale = BuildMarkScaleElement(osg::Vec3d(externNode.GetImageSize(), 0));
				pElement->LinkEndChild(pMarkScale);

				pElement->SetAttribute("path", externNode.GetImagePath().c_str());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition().vec3d());
				pElement->LinkEndChild(pMarkPosition);

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}
		return false;
	}

	bool CMarkSerialization::VisitEnter(FeExtNode::CExOverLayNode& externNode)
	{
		if (externNode.GetID() != MARK_ROOT_ID)
		{
			if (!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("OverLayMark");
				//pElement->SetAttribute("id", externNode.GetID());
				pElement->SetAttribute("title", externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				FeUtil::TiXmlElement* pMarkScale = BuildMarkScaleElement(osg::Vec3d(externNode.GetImageSize(), 0));
				pElement->LinkEndChild(pMarkScale);

// 				FeUtil::TiXmlElement* pMarkRotate = BuildMarkPostureElement(externNode.GetPosture());
// 				pElement->LinkEndChild(pMarkRotate);

// 				FeUtil::TiXmlElement* pMarkColor = BuildLineColor(externNode.GetColor());
// 				pElement->LinkEndChild(pMarkColor);

				pElement->SetAttribute("path", externNode.GetImagePath().c_str());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition().vec3d());
				pElement->LinkEndChild(pMarkPosition);

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}
		return false;
	}

	bool CMarkSerialization::VisitEnter(FeExtNode::CExSectorNode& externNode)
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("SectorMark");
				//pElement->SetAttribute("id",externNode.GetID().c_str());
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				pElement->SetDoubleAttribute("radius", externNode.GetRadius());
				pElement->SetDoubleAttribute("arcStart", externNode.GetArcStart());
				pElement->SetDoubleAttribute("arcEnd", externNode.GetArcEnd());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetLineColor());
				pElement->LinkEndChild(pLineColor);

				FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetFillColor());
				pElement->LinkEndChild(pFillColor);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition().vec3d());
				pElement->LinkEndChild(pMarkPosition);

// 				if(pElement)
// 				{
// 					double dStart = externNode.GetArcStart();
// 					double dEnd = externNode.GetArcEnd();
// 					double dRadius = externNode.GetRadius();
// 
// 					FeUtil::TiXmlElement * pVertex = new FeUtil::TiXmlElement("circleAttri");
// 					pVertex->SetDoubleAttribute("arcStart", dStart);
// 					pVertex->SetDoubleAttribute("arcEnd", dEnd);
// 					pVertex->SetDoubleAttribute("radius", dRadius);
// 					pElement->LinkEndChild(pVertex);
// 				}

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExtNode::CExArcNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("ArcMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				pElement->SetDoubleAttribute("radius", externNode.GetRadius());
				pElement->SetDoubleAttribute("arcStart", externNode.GetArcStart());
				pElement->SetDoubleAttribute("arcEnd", externNode.GetArcEnd());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetLineColor());
				pElement->LinkEndChild(pLineColor);

				FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetFillColor());
				pElement->LinkEndChild(pFillColor);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition().vec3d());
				pElement->LinkEndChild(pMarkPosition);

// 				if(pElement)
// 				{
// 					double dStart = externNode.GetArcStart();
// 					double dEnd = externNode.GetArcEnd();
// 					double dRadius = externNode.GetRadius();
// 
// 					FeUtil::TiXmlElement * pVertex = new FeUtil::TiXmlElement("circleAttri");
// 					pVertex->SetDoubleAttribute("arcStart", dStart);
// 					pVertex->SetDoubleAttribute("arcEnd", dEnd);
// 					pVertex->SetDoubleAttribute("radius", dRadius);
// 					pElement->LinkEndChild(pVertex);
// 				}

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExtNode::CExCircleNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("CircleMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				pElement->SetDoubleAttribute("radius", externNode.GetRadius());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetLineColor());
				pElement->LinkEndChild(pLineColor);

				FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetFillColor());
				pElement->LinkEndChild(pFillColor);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition().vec3d());
				pElement->LinkEndChild(pMarkPosition);

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExtNode::CExEllipseNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("EllipseMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				pElement->SetDoubleAttribute("sRadius", externNode.GetShortRadius());
				pElement->SetDoubleAttribute("lRadius", externNode.GetLongRadius());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetLineColor());
				pElement->LinkEndChild(pLineColor);

				FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetFillColor());
				pElement->LinkEndChild(pFillColor);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition().vec3d());
				pElement->LinkEndChild(pMarkPosition);

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExtNode::CExTextNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("TextMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("size", externNode.GetTextSize());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetTextColor());
				pElement->LinkEndChild(pLineColor);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition().vec3d());
				pElement->LinkEndChild(pMarkPosition);

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter( FeExtNode::CExRectNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("RectMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("line_width", externNode.GetLineWidth());

				pElement->SetDoubleAttribute("rect_width", externNode.GetWidth());
				pElement->SetDoubleAttribute("rect_height", externNode.GetHeight());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetLineColor());
				pElement->LinkEndChild(pLineColor);

				FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetFillColor());
				pElement->LinkEndChild(pFillColor);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetPosition().vec3d());
				pElement->LinkEndChild(pMarkPosition);

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter( CExStraightArrowNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("StraightArrowMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetLineColor());
				pElement->LinkEndChild(pLineColor);

				FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetFillColor());
				pElement->LinkEndChild(pFillColor);

				osg::ref_ptr<osg::Vec3dArray> vertexsArray = new osg::Vec3dArray;
				externNode.GetVertex(vertexsArray);
				BuildVertex(pElement, vertexsArray->asVector());

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter( CExAssaultNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("AssaultMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetLineColor());
				pElement->LinkEndChild(pLineColor);

				FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetFillColor());
				pElement->LinkEndChild(pFillColor);

				osg::ref_ptr<osg::Vec3dArray> vertexsArray = new osg::Vec3dArray;
				externNode.GetVertex(vertexsArray);
				BuildVertex(pElement, vertexsArray->asVector());

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter( CExAttackNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("AttackMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				pElement->SetDoubleAttribute("width", externNode.GetLineWidth());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetLineColor());
				pElement->LinkEndChild(pLineColor);

				FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetFillColor());
				pElement->LinkEndChild(pFillColor);

				osg::ref_ptr<osg::Vec3dArray> vertexsArray = new osg::Vec3dArray;
				externNode.GetVertex(vertexsArray);
				BuildVertex(pElement, vertexsArray->asVector());

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}

		return false;
	}

	void CMarkSerialization::LinkLabelItem(FeUtil::TiXmlElement* pElementParent, FeExtNode::CExLabelItemNode* pNode, int nParentID)
	{
		if(!pElementParent || !pNode)
		{
			return;
		}

		FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("LabelItem");
		pElement->SetAttribute("visible", pNode->GetVisible() ? "true" : "false");
		
		/// ID
		pElement->SetDoubleAttribute("ID", pNode->GetID());
		pElement->SetDoubleAttribute("ParentID", nParentID);

		/// 宽高
		pElement->SetDoubleAttribute("width", pNode->GetWidth());
		pElement->SetDoubleAttribute("height", pNode->GetHeight());

		/// 背景图图片路径
		pElement->SetAttribute("imagePath",  pNode->GetBackgroundImage().c_str());

		/// 文本字符串
		pElement->SetAttribute("textString",  pNode->GetText().c_str());

		/// 节点名
		pElement->SetAttribute("title",  pNode->GetName().c_str());

		/// 文本对齐方式
		pElement->SetDoubleAttribute("textAlign", pNode->GetTextAlign());

		/// 字体名称
		pElement->SetAttribute("fontName",  pNode->GetFontName().c_str());

		/// 字体大小
		pElement->SetDoubleAttribute("fontSize", pNode->GetFontSize());

		/// 文本颜色
		FeUtil::TiXmlElement* pLineColor = BuildLineColor(pNode->GetTextColor());
		pElement->LinkEndChild(pLineColor);

		/// 背景色
		FeUtil::TiXmlElement* pFillColor = BuildFillColor(pNode->GetBackgroundColor());
		pElement->LinkEndChild(pFillColor);

		pElementParent->LinkEndChild(pElement);

		if(!pNode->GetChildList().empty())
		{
			FeExtNode::CExLabelItemNodeList itemNodes = pNode->GetChildList();

			std::for_each(itemNodes.begin(), itemNodes.end(), 
				[&](osg::ref_ptr<CExLabelItemNode> nodeItem){ if(nodeItem.valid()) LinkLabelItem(pElement, nodeItem, pNode->GetID()); }
			);
		}
	}

	bool CMarkSerialization::VisitEnter( CExLabelNode& externNode )
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("LabelMark");
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				/// 绑定位置
				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetBindingPointLLH());
				pElement->LinkEndChild(pMarkPosition);

				/// ID
				pElement->SetDoubleAttribute("ID", externNode.GetID());
				pElement->SetDoubleAttribute("ParentID", 0);

				/// 宽高
				pElement->SetDoubleAttribute("width", externNode.GetWidth());
				pElement->SetDoubleAttribute("height", externNode.GetHeight());

				/// 背景图图片路径
				pElement->SetAttribute("imagePath",  externNode.GetBackgroundImage().c_str());

				/// 文本字符串
				pElement->SetAttribute("textString",  externNode.GetText().c_str());
				
				/// 节点名
				pElement->SetAttribute("title",  externNode.GetName().c_str());

				/// 文本对齐方式
				pElement->SetDoubleAttribute("textAlign", externNode.GetTextAlign());

				/// 字体名称
				pElement->SetAttribute("fontName",  externNode.GetFontName().c_str());

				/// 字体大小
				pElement->SetDoubleAttribute("fontSize", externNode.GetFontSize());

				/// 文本颜色
				FeUtil::TiXmlElement* pLineColor = BuildLineColor(externNode.GetTextColor());
				pElement->LinkEndChild(pLineColor);

				/// 背景色
				FeUtil::TiXmlElement* pFillColor = BuildFillColor(externNode.GetBackgroundColor());
				pElement->LinkEndChild(pFillColor);
				
				(*m_markList.rbegin())->LinkEndChild(pElement);

				/// 存储子节点
				if(!externNode.GetChildList().empty())
				{
					FeExtNode::CExLabelItemNodeList itemNodes = externNode.GetChildList();

					std::for_each(++itemNodes.begin(), itemNodes.end(), 
						[&](osg::ref_ptr<CExLabelItemNode> nodeItem){ if(nodeItem.valid()) LinkLabelItem(pElement, nodeItem, externNode.GetID()); }
					);
				}

				return true;
			}
		}

		return false;
	}

	bool CMarkSerialization::VisitEnter(CExParticleNode& externNode)
	{
		if(externNode.GetID() != MARK_ROOT_ID)
		{
			if(!m_markList.empty())
			{
				FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement("ParticleMark");
				pElement->SetAttribute("title",  externNode.GetName().c_str());
				pElement->SetAttribute("visible", externNode.GetVisible() ? "true" : "false");
				pElement->SetAttribute("description", externNode.GetDescription().c_str());
				pElement->SetAttribute("path", externNode.GetParticlePath().c_str());

				FeUtil::TiXmlElement* pViewPoint = BuildViewPointElement(externNode.GetViewPoint());
				pElement->LinkEndChild(pViewPoint);

				FeUtil::TiXmlElement* pMarkPosition = BuildMarkPositionElement(externNode.GetParticlePosition());
				pElement->LinkEndChild(pMarkPosition);

				FeUtil::TiXmlElement * pAngle = BuildAngleElement(externNode.GetParticleAngle());
				pElement->LinkEndChild(pAngle);

				pElement->SetAttribute("scale", externNode.GetParticleScale());

				(*m_markList.rbegin())->LinkEndChild(pElement);

				return true;

			}
		}
		return false;
	}

	bool CMarkSerialization::VisitExit( FeExtNode::CExComposeNode& externNode )
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


	FeUtil::TiXmlElement* CMarkSerialization::BuildMarkOffsetElement( const osg::Vec3d& value )
	{
		FeUtil::TiXmlElement * pElement = new FeUtil::TiXmlElement("offset");
		pElement->SetDoubleAttribute("x", value.x());
		pElement->SetDoubleAttribute("y", value.y());
		pElement->SetDoubleAttribute("z", value.z());
		return pElement;
	}

	FeUtil::TiXmlElement* CMarkSerialization::BuildMarkPositionElement( const osg::Vec3d& value )
	{
		FeUtil::TiXmlElement * pElement = new FeUtil::TiXmlElement("position");
		pElement->SetDoubleAttribute("lon", value.x());
		pElement->SetDoubleAttribute("lat", value.y());
		pElement->SetDoubleAttribute("hei", value.z());
		return pElement;
	}

	FeUtil::TiXmlElement* CMarkSerialization::BuildAngleElement(const osg::Vec3d& value)
	{
		FeUtil::TiXmlElement * pElement = new FeUtil::TiXmlElement("angle");
		pElement->SetDoubleAttribute("x", value.x());
		pElement->SetDoubleAttribute("y", value.y());
		pElement->SetDoubleAttribute("z", value.z());
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

	FeUtil::TiXmlElement* CMarkSerialization::BuildLodNodeElement( FeExtNode::CExLodModelNode::LodNodeInfo& nodeInfo , const std::string& strName)
	{
		if(!nodeInfo.node.valid())
		{
			return 0;
		}

		FeUtil::TiXmlElement* pElement = new FeUtil::TiXmlElement(strName.c_str());
		pElement->SetAttribute("title",  nodeInfo.node->GetName().c_str());
		pElement->SetAttribute("visible", nodeInfo.node->GetVisible() ? "true" : "false");
		pElement->SetAttribute("description", nodeInfo.node->GetDescription().c_str());
		
		double dMin, dMax;
		nodeInfo.node->GetRange(dMin, dMax);

		pElement->SetDoubleAttribute("min", dMin);
		pElement->SetDoubleAttribute("max", dMax);
		pElement->SetAttribute("path", nodeInfo.nodePath.c_str());

		return pElement;
	}


}
