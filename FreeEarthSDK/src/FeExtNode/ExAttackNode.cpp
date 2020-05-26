#include <FeExtNode/ExAttackNode.h>

const double PI = 3.14159265; //圆周率
const double ONE_EIGHT = 0.125; //比率(1/8)
const double ONE_FOUR = 0.25; //比率(1/4)

using namespace FeAlg;

namespace FeExtNode
{
	CExAttackNodeOption::CExAttackNodeOption()
		:CExStraightArrowNodeOption()
	{
		
	}

	CExAttackNodeOption::~CExAttackNodeOption()
	{

	}
}

namespace FeExtNode
{
	CExAttackNode::CExAttackNode(FeUtil::CRenderContext* pRender, CExAttackNodeOption* opt)
		:CExStraightArrowNode(pRender, opt)
	{
		InitFeatureNode();
	}

	CExAttackNode::~CExAttackNode()
	{

	}

	void CExAttackNode::Accept( CExternNodeVisitor& nv )
	{
		nv.VisitEnter(*this);
	}

// 	osgEarth::Features::Feature* CExAttackNode::GetFeature()
// 	{
// 		return new osgEarth::Features::Feature(new osgEarth::Symbology::LineString(), 
// 			m_opMapNode->getMapSRS());
// 	}
// 
// 	osgEarth::Style CExAttackNode::GetFeatureStyle()
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
// 		style.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = 2;
// 		return style;
// 	}

	void CExAttackNode::UpdateEditor()
	{
		/// 更新编辑器
		CExLodNode::UpdateEditor();

		if (m_rpFeature->getGeometry()->asVector().size() > 1)
		{
			m_rpLeftFeature->getGeometry()->clear();
			m_rpRightFeature->getGeometry()->clear();
			DrawAttackDirection();
		}
		//m_rpFeatureNode->init();
		m_rpLeftFeatureNode->init();
		m_rpRightFeatureNode->init();
	}

	void CExAttackNode::ReplaceBackVertex(const osg::Vec3d& vecCoord)
	{
		if(m_rpFeature.valid())
		{
			if (m_rpFeature->getGeometry()->asVector().size() == 1)
			{
				m_rpFeature->getGeometry()->asVector().push_back(vecCoord);
			}
			if (m_rpFeature->getGeometry()->asVector().size() > 1)
			{
				int nIndex = m_rpFeature->getGeometry()->asVector().size() - 1;
				m_rpFeature->getGeometry()->asVector()[nIndex] = vecCoord;

				m_rpLeftFeature->getGeometry()->clear();
				m_rpRightFeature->getGeometry()->clear();
				DrawAttackDirection();
			}
			
			//m_rpFeatureNode->init();
			m_rpLeftFeatureNode->init();
			m_rpRightFeatureNode->init();
			UpdateEditor();
		}
	}

	void CExAttackNode::PushBackVertex( const osg::Vec3d& vecCoord )
	{ 
		if(m_rpFeature.valid())
		{
			m_rpFeature->getGeometry()->asVector().push_back(vecCoord);

			//m_rpFeatureNode->init();
			//UpdateEditor();
			RebuildEditPoints();
		}
	}

	void CExAttackNode::DrawAttackDirection()
	{
		if (!m_vecControl.empty())
		{
			m_vecControl.clear();
		}

		for (int i=0; i<m_rpFeature->getGeometry()->asVector().size(); i++)
		{
			m_vecControl.push_back(m_rpFeature->getGeometry()->asVector()[i]);
		}
		FeAlg::CBSplineAlgorithm bcurve(m_vecControl);
		bcurve.Computeinterpolation();

		std::vector<osg::Vec3d> vecBSplinePoints; //BSpline曲线上的点
		vecBSplinePoints = bcurve.m_vecInterpolatioNode;

		ComputeSumLegth();
		ComputeArrowTail(vecBSplinePoints);
		ComputeArrowBody(vecBSplinePoints);
		ComputeArrowHead(vecBSplinePoints);
	}

	void CExAttackNode::ComputeArrowTail(const std::vector<osg::Vec3d> &BSplinePoints)
	{
		if (BSplinePoints.empty())
		{
			return;
		}

		m_dDistance = m_dSumLength * ONE_EIGHT * ONE_FOUR;

		osg::Vec3d firstPoint = BSplinePoints.at(0);
		osg::Vec3d nextPoint = BSplinePoints.at(1);

		double angle = 0.0;
		FeAlg::CalculateYawRadian(NULL, nextPoint, firstPoint, angle);
		angle = angle + 1.5*PI;

		double x = firstPoint.x() + m_dDistance * cos(angle); //求得箭尾顶点坐标
		double y = firstPoint.y() + m_dDistance * sin(angle);

		osg::Vec3d tailPoint(x, y, 0);
		m_rpLeftFeature->getGeometry()->asVector().push_back(tailPoint);
		m_rpRightFeature->getGeometry()->asVector().push_back(tailPoint);
	}

	void CExAttackNode::ComputeArrowBody(const std::vector<osg::Vec3d> &BSplinePoints)
	{
		if (BSplinePoints.empty())
		{
			return;
		}

		m_dDistance = m_dSumLength * ONE_EIGHT * ONE_FOUR;

		//箭身所有点到等距离的点的坐标
		for (int i=0; i<(BSplinePoints.size() - 1); i+=3)
		{
			osg::Vec3d frontPoint = BSplinePoints.at(i);
			osg::Vec3d nextPoint = BSplinePoints.at(i+1);

			double angle = 0.0;
			FeAlg::CalculateYawRadian(NULL, frontPoint, nextPoint, angle);
			angle = angle + 1.5*PI;
			double tempLen = (m_dDistance/300)*(BSplinePoints.size()-1-i);

			double x1 = frontPoint.x() + (m_dDistance + tempLen) * cos(angle - (PI/2)); //求得箭尾点1坐标
			double y1 = frontPoint.y() + (m_dDistance + tempLen) * sin(angle - (PI/2));
			double x2 = frontPoint.x() + (m_dDistance + tempLen) * cos(angle + (PI/2));//求得箭尾点2坐标
			double y2 = frontPoint.y() + (m_dDistance + tempLen) * sin(angle + (PI/2));

			osg::Vec3d tempLeftPoint(x1, y1, 0);
			osg::Vec3d tempRightPoint(x2, y2, 0);
			m_rpLeftFeature->getGeometry()->asVector().push_back(tempLeftPoint);
			m_rpRightFeature->getGeometry()->asVector().push_back(tempRightPoint);
		}
	}

	void CExAttackNode::ComputeArrowHead(const std::vector<osg::Vec3d> &BSplinePoints)
	{
		if (BSplinePoints.empty())
		{
			return;
		}

		m_dDistance = m_dSumLength * ONE_EIGHT * ONE_FOUR;

		//计算箭头顶点坐标
		osg::Vec3d lastPoint = BSplinePoints.at(BSplinePoints.size() - 1);
		osg::Vec3d frontPoint = BSplinePoints.at(BSplinePoints.size() - 2);

		double angle = 0.0;
		FeAlg::CalculateYawRadian(NULL, lastPoint, frontPoint, angle);
		angle = angle + 0.5*PI;

		double x = lastPoint.x() - (m_dDistance * 3) * cos(angle); //求得箭头顶点坐标
		double y = lastPoint.y() - (m_dDistance * 3) * sin(angle);
		osg::Vec3d headPoint(x, y, 0);

		//计算外箭头坐标
		double outArrow_lenght = m_dDistance * 2; //外箭头长度
		const double OUT_ARROW_DEGREES = PI/3; //外箭头角度60°

		double x1 = lastPoint.x() + outArrow_lenght * cos(angle - OUT_ARROW_DEGREES); //求得外箭头点1坐标
		double y1 = lastPoint.y() + outArrow_lenght * sin(angle - OUT_ARROW_DEGREES);
		double x2 = lastPoint.x() + outArrow_lenght * cos(angle + OUT_ARROW_DEGREES); //求得外箭头点2坐标
		double y2 = lastPoint.y() + outArrow_lenght * sin(angle + OUT_ARROW_DEGREES);

		osg::Vec3d leftOutPoint(x1, y1, 0);
		osg::Vec3d rightOutPoint(x2, y2, 0);
		m_rpLeftFeature->getGeometry()->asVector().push_back(leftOutPoint);
		m_rpLeftFeature->getGeometry()->asVector().push_back(headPoint);
		m_rpRightFeature->getGeometry()->asVector().push_back(rightOutPoint);
		m_rpRightFeature->getGeometry()->asVector().push_back(headPoint);
	}

	void CExAttackNode::ComputeSumLegth()
	{
		if (m_vecControl.empty())
		{
			return;
		}
		m_dSumLength = 0;
		for (int i=0; i<(m_vecControl.size() - 1); i++)
		{
			m_dSumLength += FeAlg::ComputeDistance(m_vecControl.at(i), m_vecControl.at(i+1));
		}
	}

}