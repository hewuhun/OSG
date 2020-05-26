#include <FeExtNode/ExStraightArrowNode.h>

const double PI = 3.14159265; //圆周率

namespace FeExtNode
{
	CExStraightArrowNodeOption::CExStraightArrowNodeOption()
		:CExFeatureNodeOption()
	{

	}

	CExStraightArrowNodeOption::~CExStraightArrowNodeOption()
	{

	}

}

namespace FeExtNode
{
	CExStraightArrowNode::CExStraightArrowNode(FeUtil::CRenderContext* pRender, CExStraightArrowNodeOption* opt)
		: CExFeatureNode(pRender, opt)
	{
		InitFeatureNode();
		m_bMoveFlag = true;
	}

	CExStraightArrowNode::~CExStraightArrowNode(void)
	{
	}

	osgEarth::Features::Feature* CExStraightArrowNode::GetFeature()
	{
		return new osgEarth::Features::Feature(new osgEarth::Symbology::LineString(), 
			m_opMapNode->getMapSRS());
	}

	bool CExStraightArrowNode::InitFeatureNode()
	{
		if(m_rpFeatureNode.valid() && m_rpLeftFeatureNode.valid() && m_rpRightFeatureNode.valid())
		{
			/// 已存在则先移除，防止多次调用生成多个节点
			this->removeChild(m_rpFeatureNode);
			this->removeChild(m_rpLeftFeatureNode);
			this->removeChild(m_rpRightFeatureNode);
		}

		m_rpFeature = GetFeature();
		m_rpLeftFeature = GetFeature();
		m_rpRightFeature = GetFeature();
		m_rpFeatureNode = new osgEarth::Annotation::FeatureNode(m_opMapNode.get(), m_rpFeature);
		m_rpLeftFeatureNode = new osgEarth::Annotation::FeatureNode(m_opMapNode.get(), m_rpLeftFeature);
		m_rpRightFeatureNode = new osgEarth::Annotation::FeatureNode(m_opMapNode.get(), m_rpRightFeature);

		if(m_rpLeftFeature && m_rpRightFeature)
		{
			m_rpLeftFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			m_rpLeftFeatureNode->setStyle(GetFeatureStyle());
			m_rpLeftFeatureNode->setDynamic(true);
			addChild(m_rpLeftFeatureNode.get());

			m_rpRightFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			m_rpRightFeatureNode->setStyle(GetFeatureStyle());
			m_rpRightFeatureNode->setDynamic(true);
			addChild(m_rpRightFeatureNode.get());

			m_rpFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			m_rpFeatureNode->setStyle(GetFeatureStyle());
			m_rpFeatureNode->setDynamic(true);
			//addChild(m_rpFeatureNode.get());

			return true;
		}

		return false;
	}

	osgEarth::Style CExStraightArrowNode::GetFeatureStyle()
	{
		osgEarth::Style style;
		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping()
			= osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique()
			= osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_GPU;
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->widthUnits()
			= osgEarth::Symbology::Units::PIXELS;
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->tessellation() = 100;
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->stipple() = GetLineStipple();
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = osgEarth::Symbology::Color(GetLineColor());
		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = GetLineWidth();
		return style;
	}

	CExStraightArrowNodeOption* CExStraightArrowNode::GetOption()
	{
		return dynamic_cast<CExStraightArrowNodeOption*>(m_rpOptions.get());
	}

	void CExStraightArrowNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

	void CExStraightArrowNode::UpdateEditor()
	{
		/// 更新编辑器
		CExLodNode::UpdateEditor();

		if (m_rpFeature->getGeometry()->asVector().size() == 2)
		{
			m_rpLeftFeature->getGeometry()->clear();
			m_rpRightFeature->getGeometry()->clear();
			DrawStraightArrow();
		}
		m_rpFeatureNode->init();
		m_rpLeftFeatureNode->init();
		m_rpRightFeatureNode->init();
	}

	void CExStraightArrowNode::SetVertex( osg::Vec3dArray* pCoord )
	{
		CExFeatureNode::SetVertex(pCoord);

		if(m_rpFeatureNode.valid() && m_rpLeftFeature.valid() && m_rpRightFeature.valid())
		{
			m_rpLeftFeature->getGeometry()->clear();
			m_rpRightFeature->getGeometry()->clear();
			DrawInternalExtra();
			m_rpFeatureNode->init();
			m_rpLeftFeatureNode->init();
			m_rpRightFeatureNode->init();
		}
	}

	void CExStraightArrowNode::SetLineColor( const osg::Vec4d& vecLineColor )
	{
		if (GetLineColor() != vecLineColor)
		{
			GetOption()->lineColor() = vecLineColor;

			osgEarth::Style style = m_rpFeatureNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = 
				osgEarth::Symbology::Color(vecLineColor);

			m_rpFeatureNode->setStyle(style);
			m_rpFeatureNode->init();
			m_rpLeftFeatureNode->setStyle(style);
			m_rpLeftFeatureNode->init();
			m_rpRightFeatureNode->setStyle(style);
			m_rpRightFeatureNode->init();
		}
	}

	void CExStraightArrowNode::SetLineWidth( const int& nLineWidth )
	{
		if (GetLineWidth() != nLineWidth)
		{
			GetOption()->lineWidth() = nLineWidth;

			osgEarth::Style style = m_rpFeatureNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = nLineWidth;

			m_rpFeatureNode->setStyle(style);
			m_rpFeatureNode->init();
			m_rpLeftFeatureNode->setStyle(style);
			m_rpLeftFeatureNode->init();
			m_rpRightFeatureNode->setStyle(style);
			m_rpRightFeatureNode->init();
		}
	}

	void CExStraightArrowNode::SetLineStipple( const unsigned int& unLineStipple )
	{
		if (GetLineStipple() != unLineStipple)
		{
			GetOption()->lineStipple() = unLineStipple;

			osgEarth::Style style = m_rpFeatureNode->getStyle();
			style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->stipple() = unLineStipple;

			m_rpFeatureNode->setStyle(style);
			m_rpFeatureNode->init();
			m_rpLeftFeatureNode->setStyle(style);
			m_rpLeftFeatureNode->init();
			m_rpRightFeatureNode->setStyle(style);
			m_rpRightFeatureNode->init();
		}
	}

	void CExStraightArrowNode::ReplaceBackVertex(const osg::Vec3d& vecCoord)
	{
		if(m_rpFeature.valid())
		{
			if (m_rpFeature->getGeometry()->asVector().size() == 1)
			{
				m_rpFeature->getGeometry()->asVector().push_back(vecCoord);
			}
			if (m_rpFeature->getGeometry()->asVector().size() == 2 && m_bMoveFlag)
			{
				int nIndex = m_rpFeature->getGeometry()->asVector().size() - 1;
				m_rpFeature->getGeometry()->asVector()[nIndex] = vecCoord;

				m_rpLeftFeature->getGeometry()->clear();
				m_rpRightFeature->getGeometry()->clear();
				DrawStraightArrow();
			}
			
			m_rpFeatureNode->init();
			m_rpLeftFeatureNode->init();
			m_rpRightFeatureNode->init();
			UpdateEditor();
		}
	}

	void CExStraightArrowNode::PushBackVertex( const osg::Vec3d& vecCoord )
	{ 
		if(m_rpFeature.valid())
		{
			if (m_rpFeature->getGeometry()->asVector().size() == 0)
			{
				m_rpFeature->getGeometry()->asVector().push_back(vecCoord);
			}
			if (m_rpFeature->getGeometry()->asVector().size() == 2)
			{
				m_bMoveFlag = false;
			}

			m_rpFeatureNode->init();
			//UpdateEditor();
			RebuildEditPoints();
		}
	}

	void CExStraightArrowNode::DrawStraightArrow()
	{
		osg::Vec3d startPoint; //直箭头开始控制点
		osg::Vec3d endPoint; //直箭头结束控制点
		osg::Vec3d leftPoint;
		osg::Vec3d rightPoint;

		startPoint = m_rpFeature->getGeometry()->asVector().front();
		endPoint = m_rpFeature->getGeometry()->asVector().back();

		ComputeCoordinate(startPoint, endPoint, leftPoint, rightPoint);

		m_rpLeftFeature->getGeometry()->asVector().push_back(startPoint);
		m_rpLeftFeature->getGeometry()->asVector().push_back(endPoint);
		m_rpLeftFeature->getGeometry()->asVector().push_back(leftPoint);

		m_rpRightFeature->getGeometry()->asVector().push_back(endPoint);
		m_rpRightFeature->getGeometry()->asVector().push_back(rightPoint);
	}

	void CExStraightArrowNode::ComputeCoordinate(const osg::Vec3d startPoint, const osg::Vec3d endPoint, osg::Vec3d &leftPoint, osg::Vec3d &rightPoint)
	{
		if (startPoint.valid() && endPoint.valid())
		{
			const double ONE_SIX_RATIO = 0.16666667; //箭头长度占中轴线长度的比率(1/6)
			const double ARROW_DEGREES = PI/6; //箭头角度30°

			double distance = FeAlg::ComputeDistance(startPoint, endPoint);
			double arrow_lenght = distance * ONE_SIX_RATIO; //箭头长度
			double angle = 0.0;
			FeAlg::CalculateYawRadian(NULL, startPoint, endPoint, angle);
			angle = angle + 1.5*PI;

			leftPoint.x() = endPoint.x() + arrow_lenght * cos(angle - ARROW_DEGREES); //求得箭头点1坐标
			leftPoint.y() = endPoint.y() + arrow_lenght * sin(angle - ARROW_DEGREES);
			rightPoint.x() = endPoint.x() + arrow_lenght * cos(angle + ARROW_DEGREES); //求得箭头点2坐标
			rightPoint.y() = endPoint.y() + arrow_lenght * sin(angle + ARROW_DEGREES);
		}
	}

}
