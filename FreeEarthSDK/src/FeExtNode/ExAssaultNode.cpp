#include <FeExtNode/ExAssaultNode.h>

const double PI = 3.14159265; //圆周率

namespace FeExtNode
{
	CExAssaultNodeOption::CExAssaultNodeOption()
		:CExStraightArrowNodeOption()
	{

	}

	CExAssaultNodeOption::~CExAssaultNodeOption()
	{

	}

}

namespace FeExtNode
{
	CExAssaultNode::CExAssaultNode(FeUtil::CRenderContext* pRender, CExAssaultNodeOption* opt)
		:CExStraightArrowNode(pRender, opt)
	{
		InitFeatureNode();
		m_bMoveFlag = true;
	}

	CExAssaultNode::~CExAssaultNode()
	{

	}

	void CExAssaultNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

// 	osgEarth::Features::Feature* CExAssaultNode::GetFeature()
// 	{
// 		return new osgEarth::Features::Feature(new osgEarth::Symbology::LineString(), 
// 			m_opMapNode->getMapSRS());
// 	}
// 
// 	osgEarth::Style CExAssaultNode::GetFeatureStyle()
// 	{
// 		osgEarth::Style style;
// 		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping()
// 			= osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
// 		style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique()
// 			= osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_GPU;
// 		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->widthUnits()
// 			= osgEarth::Symbology::Units::PIXELS;
// 		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->tessellation() = 100;
// 		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->stipple() = GetLineStipple();
// 		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = osgEarth::Symbology::Color(osg::Vec4d(0.0, 0.0, 1.0, 1.0));
// 		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = GetLineWidth();
// 		return style;
// 	}

	void CExAssaultNode::UpdateEditor()
	{
		/// 更新编辑器
		CExLodNode::UpdateEditor();

		if (m_rpFeature->getGeometry()->asVector().size() == 2)
		{
			m_rpLeftFeature->getGeometry()->clear();
			m_rpRightFeature->getGeometry()->clear();
			DrawAssaultDirection();
		}
		m_rpFeatureNode->init();
		m_rpLeftFeatureNode->init();
		m_rpRightFeatureNode->init();
	}

	void CExAssaultNode::ReplaceBackVertex(const osg::Vec3d& vecCoord)
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
				DrawAssaultDirection();
			}

			m_rpFeatureNode->init();
			m_rpLeftFeatureNode->init();
			m_rpRightFeatureNode->init();
			UpdateEditor();
		}
	}

	void CExAssaultNode::PushBackVertex( const osg::Vec3d& vecCoord )
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

	void CExAssaultNode::DrawAssaultDirection()
	{
		osg::Vec3d startPoint; //突击方向开始控制点
		osg::Vec3d endPoint; //突击方向结束控制点
		osg::Vec3d leftOutPoint; //箭头外侧控制点
		osg::Vec3d rightOutPoint;
		osg::Vec3d leftInPoint; //箭头内侧控制点
		osg::Vec3d rightInPoint;
		osg::Vec3d leftTailPoint; //箭尾控制点
		osg::Vec3d rightTailPoint;

		startPoint = m_rpFeature->getGeometry()->asVector().front();
		endPoint = m_rpFeature->getGeometry()->asVector().back();

		//突击方向箭头外侧控制点坐标
		ComputeCoordinate(OUT_ARROW, startPoint, endPoint, leftOutPoint, rightOutPoint);
		//突击方向箭头内侧控制点坐标
		ComputeCoordinate(IN_ARROW, startPoint, endPoint, leftInPoint, rightInPoint);
		//突击方向箭尾的控制点坐标
		ComputeCoordinate(TAIL_ARROW, startPoint, endPoint, leftTailPoint, rightTailPoint);

		m_rpLeftFeature->getGeometry()->asVector().push_back(endPoint);
		m_rpLeftFeature->getGeometry()->asVector().push_back(leftOutPoint);
		m_rpLeftFeature->getGeometry()->asVector().push_back(leftInPoint);
		m_rpLeftFeature->getGeometry()->asVector().push_back(leftTailPoint);
		m_rpLeftFeature->getGeometry()->asVector().push_back(startPoint);

		m_rpRightFeature->getGeometry()->asVector().push_back(endPoint);
		m_rpRightFeature->getGeometry()->asVector().push_back(rightOutPoint);
		m_rpRightFeature->getGeometry()->asVector().push_back(rightInPoint);
		m_rpRightFeature->getGeometry()->asVector().push_back(rightTailPoint);
		m_rpRightFeature->getGeometry()->asVector().push_back(startPoint);
	}

	void CExAssaultNode::ComputeCoordinate(int nFlag, const osg::Vec3d startPoint, const osg::Vec3d endPoint, osg::Vec3d &leftPoint, osg::Vec3d &rightPoint)
	{
		if (startPoint.valid() && endPoint.valid())
		{
			const double ONE_TWO_RATIO = 0.5; //比率(1/2)
			const double ONE_THREE_RATIO = 0.33333333; //比率(1/3)
			const double ONE_FOUR_RATIO = 0.25; //比率(1/4)
			const double OUT_ARROW_DEGREES = PI/3; //外箭头角度60°
			const double IN_ARROW_DEGREES = PI/4; //内箭头角度45°

			double angle = 0.0;
			FeAlg::CalculateYawRadian(NULL, startPoint, endPoint, angle);
			angle = angle + 1.5*PI;
			double distance = FeAlg::ComputeDistance(startPoint, endPoint);

			if (nFlag == 0)
			{
				double outArrow_lenght = distance * ONE_FOUR_RATIO; //外箭头长度

				leftPoint.x() = endPoint.x() + outArrow_lenght * cos(angle - OUT_ARROW_DEGREES); //求得外箭头点1坐标
				leftPoint.y() = endPoint.y() + outArrow_lenght * sin(angle - OUT_ARROW_DEGREES);
				rightPoint.x() = endPoint.x() + outArrow_lenght * cos(angle + OUT_ARROW_DEGREES); //求得外箭头点2坐标
				rightPoint.y() = endPoint.y() + outArrow_lenght * sin(angle + OUT_ARROW_DEGREES);
			}
			if (nFlag == 1)
			{
				//内箭头长度 = ( cos(60)/cos(45) ) * 外箭头长度，cos(60)/cos(45) = 0.70710678
				double inArrow_lenght = (distance * ONE_FOUR_RATIO) * 0.70710678;//内箭头长度

				leftPoint.x() = endPoint.x() + inArrow_lenght * cos(angle - IN_ARROW_DEGREES); //求得内箭头点1坐标
				leftPoint.y() = endPoint.y() + inArrow_lenght * sin(angle - IN_ARROW_DEGREES);
				rightPoint.x() = endPoint.x() + inArrow_lenght * cos(angle + IN_ARROW_DEGREES); //求得内箭头点2坐标
				rightPoint.y() = endPoint.y() + inArrow_lenght * sin(angle + IN_ARROW_DEGREES);
			}
			if (nFlag == 2)
			{
				double tailArrow_lenght = (distance * ONE_THREE_RATIO) * ONE_TWO_RATIO; //箭尾1/2的长度

				leftPoint.x() = startPoint.x() + tailArrow_lenght * cos(angle - (PI/2)); //求得箭尾点1坐标
				leftPoint.y() = startPoint.y() + tailArrow_lenght * sin(angle - (PI/2));
				rightPoint.x() = startPoint.x() + tailArrow_lenght * cos(angle + (PI/2));//求得箭尾点2坐标
				rightPoint.y() = startPoint.y() + tailArrow_lenght * sin(angle + (PI/2));
			}
		}
	}

}

