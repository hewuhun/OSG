#include <FeMeasure/RadialLineOfSightNode.h>
#include <FeMeasure/DPLineSegmentIntersector.h>

#include <osg/ShapeDrawable>
#include <osg/Depth>
#include <osg/AutoTransform>
#include <osgEarth/GeoMath>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/MathUtil.h>
#include <osgDB/ConvertUTF>

#include <map>
#include <vector>

namespace FeMeasure
{
	CRadialLineOfSightNode::CRadialLineOfSightNode(osgEarth::MapNode *mapNode)
		: m_opMapNode(mapNode)
		, m_v3dEndLLH(0, 0, 0.0)
		, m_dViewHeight(3.0)
		, m_rpOutlineFeatureNode(NULL)
		, m_iNumSpokes(150)
		, m_iDetectNum(50)
		, m_bComputeCircleGrid(false)
		, m_bComputeViewLine(false)
		, m_rpMeasureInfoDisplay(NULL)
	{
		m_iDrawCur = m_iNumSpokes;
		m_iDrawStart = 0;
		//getOrCreateStateSet()->setRenderBinDetails(1, "RenderBin");
		CreateOutlineElement();
		CreateViewLineElement();
		CreateMouseLineElement();
		CreateGridElement();

		m_rpRadialCallBack = new CRadialLineCallBack(this);
		setUpdateCallback(m_rpRadialCallBack);
	}

	CRadialLineOfSightNode::~CRadialLineOfSightNode()
	{
		m_opMapNode = NULL;
	}

	void CRadialLineOfSightNode::SetCenter(const osgEarth::GeoPoint& center)
	{
		if (m_geoCenter != center)
		{
			m_geoCenter = center;
			ComputeOutline();
			//ComputeViewLine();
			osgEarth::GeoPoint viewPoint(m_opMapNode->getMapSRS(), center.x(), center.y(), m_dViewHeight, osgEarth::ALTMODE_RELATIVE);
			if (!m_rpMeasureInfoDisplay.valid())
			{
				m_rpMeasureInfoDisplay = new FeMeasure::CDisplayByBillBoard(m_opMapNode.get());
				addChild(m_rpMeasureInfoDisplay->GetDisplayAsOsgNode());
			}

			char cTemp[100];
			std::sprintf(cTemp, "视高: %.3f米", m_dViewHeight);
			m_rpMeasureInfoDisplay->SetOrCreateString(0,
				osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
			std::sprintf(cTemp, "经度: %.6f度", center.x());
			m_rpMeasureInfoDisplay->SetOrCreateString(2,
				osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
			std::sprintf(cTemp, "纬度: %.6f度", center.y());
			m_rpMeasureInfoDisplay->SetOrCreateString(3,
				osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));

			viewPoint.makeAbsolute(m_opMapNode->getTerrain());
			m_rpMeasureInfoDisplay->SetPosition(viewPoint.vec3d());
		}
	}

	const osgEarth::GeoPoint& CRadialLineOfSightNode::GetCenter()
	{
		return m_geoCenter;
	}

	void CRadialLineOfSightNode::SetViewHeight(double height)
	{
		if (m_dViewHeight != height)
		{
			m_dViewHeight = height;
			//ComputeViewLine();
		}
	}

	double CRadialLineOfSightNode::GetViewHeight()
	{
		return m_dViewHeight;
	}

	void CRadialLineOfSightNode::CreateOutlineElement()
	{
		m_rpOutlineFeature = new osgEarth::Features::Feature(new osgEarth::Features::Ring(), m_opMapNode->getMapSRS());   // 建立一个环几何体。
		m_rpOutlineFeature->geoInterp() = osgEarth::GEOINTERP_GREAT_CIRCLE;	// 两点之间用直线插值

		osgEarth::Symbology::AltitudeSymbol* pAlt = m_rpOutlineFeature->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
		pAlt->clamping() = pAlt->CLAMP_TO_TERRAIN;			// 绘制的线沿着地表
		pAlt->technique() = pAlt->TECHNIQUE_SCENE;

		//osgEarth::Symbology::PointSymbol * pPS = m_rpOutlineFeature->style()->getOrCreate<osgEarth::Symbology::PointSymbol>();
		//pPS->fill()->color() = osgEarth::Symbology::Color::Yellow;
		//pPS->size() = 3.0f;

		// define a style for the line
		osgEarth::Symbology::LineSymbol* pLs = m_rpOutlineFeature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
		pLs->stroke()->color() = osgEarth::Symbology::Color::Yellow;
		pLs->stroke()->width() = 3.0f;
		pLs->tessellation() = 0;						// 两点之间，分段的个数
		//pLs->stroke()->widthUnits() = osgEarth::Symbology::Units::METERS;  // default is pixels, 输入的宽度是像素的宽度
		//pLs->stroke()->lineCap() = osgEarth::Symbology::Stroke::LINECAP_SQUARE;   // 直线尽头的扩展的样式
		//pLs->stroke()->lineJoin() = osgEarth::Symbology::Stroke::LINEJOIN_ROUND;	  // 直线转弯处圆滑处理

		osgEarth::Symbology::Style style;
		osgEarth::Features::GeometryCompilerOptions options;
		options.maxGranularity() = 180.0;
		m_rpOutlineFeatureNode = new osgEarth::Annotation::FeatureNode(m_opMapNode.get(), m_rpOutlineFeature.get(), style, options);
		m_rpOutlineFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		m_rpOutlineFeatureNode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		m_rpOutlineFeatureNode->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		//m_rpOutlineFeatureNode->getOrCreateStateSet()->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON);
		//m_rpOutlineFeatureNode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		m_rpOutlineFeatureNode->setClusterCulling(false);
		addChild(m_rpOutlineFeatureNode.get());
	}

	void CRadialLineOfSightNode::ComputeOutline()
	{
		if (!m_opMapNode.valid() || !m_rpOutlineFeatureNode.valid())
		{
			return;
		}

		osgEarth::GeoPoint centerMap;
		m_geoCenter.transform(m_opMapNode->getMapSRS(), centerMap);
		osg::Vec3d v3dCenterWorld;
		centerMap.toWorld(v3dCenterWorld, m_opMapNode->getTerrain());

		bool isProjected = m_opMapNode->getMapSRS()->isProjected();
		osg::Vec3d up = isProjected ? osg::Vec3d(0, 0, 1) : osg::Vec3d(v3dCenterWorld);
		up.normalize();

		//Get the "side" vector
		osg::Vec3d side = isProjected ? osg::Vec3d(1, 0, 0) : up ^ osg::Vec3d(0, 0, 1);

		//Get the number of spokes
		double delta = osg::PI * 2.0 / (double)m_iNumSpokes;

		m_rpOutlineFeature->getGeometry()->clear();
		double dRadius = 1.0;
		osg::Vec3d vecLLH;

		m_vecData.clear();
		for (unsigned int i = 0; i < (unsigned int)m_iNumSpokes; i++)
		{
			double angle = delta * (double)i;
			osg::Quat quat(angle, up);
			osg::Vec3d spoke = quat * (side * dRadius);
			osg::Vec3d end = v3dCenterWorld + spoke;
			m_opMapNode->getMapSRS()->getEllipsoid()->convertXYZToLatLongHeight(
				end.x(), end.y(), end.z(), vecLLH.y(), vecLLH.x(), vecLLH.z());
			vecLLH.x() = osg::RadiansToDegrees(vecLLH.x());
			vecLLH.y() = osg::RadiansToDegrees(vecLLH.y());

			m_rpOutlineFeature->getGeometry()->push_back(vecLLH.x(), vecLLH.y());
			vecLLH.z() = 0;
			m_vecData.push_back(vecLLH);
		}

		m_rpOutlineFeatureNode->init();

		/**/
		if (m_rpViewFeatureNode.valid())
		{
			m_rpViewFeature->getGeometry()->clear();
			m_rpViewFeatureNode->init();
			m_bComputeViewLine = false;
		}
	}

	void CRadialLineOfSightNode::UpdateOutline()
	{
		if (!m_opMapNode.valid() || !m_rpOutlineFeatureNode.valid() || m_iNumSpokes != m_rpOutlineFeature->getGeometry()->size())
		{
			return;
		}

		osg::Vec3d center = m_geoCenter.vec3d(); center.z() = 0;
		double newLen = (m_v3dEndLLH - center).length();

		for (unsigned int i = 0; i < (unsigned int)m_iNumSpokes; ++i)
		{
			osg::Vec3d curVector = m_rpOutlineFeature->getGeometry()->at(i) - center;
			if (FeMath::EqualZero(curVector.length()))
			{
				curVector = m_vecData.at(i) - center;
			}
			double radio = newLen / curVector.length();
			(*m_rpOutlineFeature->getGeometry())[i] = center + (curVector * radio);
			//(*m_rpOutlineFeature->getGeometry())[i].z() = 0.0;
		}

		m_rpOutlineFeatureNode->init();
	}

	void CRadialLineOfSightNode::CreateViewLineElement()
	{
		m_rpViewFeature= new osgEarth::Features::Feature(new osgEarth::Features::LineString(), m_opMapNode->getMapSRS());   // 建立一个环几何体。
		m_rpViewFeature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;

		osgEarth::Symbology::AltitudeSymbol* pAlt = m_rpViewFeature->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
		pAlt->clamping() = pAlt->CLAMP_ABSOLUTE;			// 绘制的线沿着地表
		pAlt->technique() = pAlt->TECHNIQUE_GPU;				// 利用gpu绘制

		// define a style for the line
		osgEarth::Symbology::LineSymbol* pLs = m_rpViewFeature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
		pLs->stroke()->color() = osgEarth::Symbology::Color::Cyan;
		pLs->stroke()->width() = 2.0f;
		pLs->tessellation() = 0;						// 两点之间，分段的个数

		osgEarth::Symbology::Style style;
		osgEarth::Features::GeometryCompilerOptions options;
		options.maxGranularity() = 180.0;
		m_rpViewFeatureNode= new osgEarth::Annotation::FeatureNode(m_opMapNode.get(), m_rpViewFeature.get(), style, options);
		m_rpViewFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		m_rpViewFeatureNode->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		addChild(m_rpViewFeatureNode.get());

		osg::Sphere* shape = new osg::Sphere(osg::Vec3(0, 0, 0), 1.0f);
		osg::Geode* geode = new osg::Geode();
		osg::ShapeDrawable* _shapeDrawable = new osg::ShapeDrawable(shape);
		_shapeDrawable->setDataVariance(osg::Object::DYNAMIC);
		_shapeDrawable->setColor(osg::Vec4d(1.0f, 0.0f, 1.0f, 1.0f));
		geode->addDrawable(_shapeDrawable);

		geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
		geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

		osg::MatrixTransform* _scaler = new osg::MatrixTransform;
		_scaler->setMatrix(osg::Matrixd::scale(3.0, 3.0, 3.0));
		_scaler->addChild(geode);

		osg::AutoTransform* at = new osg::AutoTransform;
		at->setAutoScaleToScreen(true);
		at->addChild(_scaler);

		m_rpViewSphere = new osg::MatrixTransform;
		m_rpViewSphere->addChild(at);
		addChild(m_rpViewSphere);
	}

	void CRadialLineOfSightNode::ComputeViewLine()
	{
		if (m_bComputeViewLine && m_rpViewFeatureNode.valid())
		{
			double zValue;
			m_opMapNode->getTerrain()->getHeight(m_opMapNode->getMapSRS(), m_geoCenter.x(), m_geoCenter.y(), &zValue);

			m_rpViewFeature->getGeometry()->clear();
			m_rpViewFeature->getGeometry()->push_back(osg::Vec3d(m_geoCenter.x(), m_geoCenter.y(), zValue));
			m_rpViewFeature->getGeometry()->push_back(osg::Vec3d(m_geoCenter.x(), m_geoCenter.y(), zValue + m_dViewHeight));

			if (m_rpViewSphere.valid())
			{
				osg::Matrix matrix;
				m_opMapNode->getMapSRS()->getEllipsoid()->
					computeLocalToWorldTransformFromLatLongHeight(osg::DegreesToRadians(m_geoCenter.y()), osg::DegreesToRadians(m_geoCenter.x()),
					zValue + m_dViewHeight, matrix);
				m_rpViewSphere->setMatrix(matrix);
			}

			m_rpViewFeatureNode->init();
		}
	}

	void CRadialLineOfSightNode::SetEndLLH(osg::Vec3d endLLH)
	{
		if (!m_rpOutlineFeature->getGeometry()->empty())
		{
			m_v3dEndLLH = endLLH;
			m_v3dEndLLH.z() = 0.0;
			UpdateOutline();
			UpdateMouseLine();
			m_bComputeViewLine = true;

			double dDistance = osgEarth::GeoMath::rhumbDistance(
				osg::DegreesToRadians(m_geoCenter.y()),
				osg::DegreesToRadians(m_geoCenter.x()),
				osg::DegreesToRadians(m_v3dEndLLH.y()),
				osg::DegreesToRadians(m_v3dEndLLH.x()));

			if (m_rpMeasureInfoDisplay.valid())
			{
				char cTemp[100];
				std::sprintf(cTemp, "半径: %.3f米", dDistance);
				m_rpMeasureInfoDisplay->SetOrCreateString(1,
					osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
			}
		}
	}

	const osg::Vec3d& CRadialLineOfSightNode::GetEndLLH()
	{
		return m_v3dEndLLH;
	}

	void CRadialLineOfSightNode::CreateGridElement()
	{
		RebuildGreenGeometry();
		RebuildRedGeometry();
	}

	void CRadialLineOfSightNode::ComputeGrid()
	{
		if (!m_rpGridFeatureNodeGreen.valid() 
			|| !m_rpGridFeatureNodeRed.valid() 
			|| !m_rpMouseLineFeatureNode.valid() 
			|| m_iNumSpokes != m_rpOutlineFeature->getGeometry()->size())
		{
			return;
		}

		m_rpMouseLineFeature->getGeometry()->clear();
		m_rpMouseLineFeatureNode->init();
		m_rpGridMultiGeometryGreen->getComponents().clear();
		m_rpGridFeatureNodeGreen->init();
		m_rpGridMultiGeometryRed->getComponents().clear();
		m_rpGridFeatureNodeRed->init();

		m_vecColorToPos.clear();

		std::map<double, int> mapDisToPos;
		for (unsigned int i = 0; i < (unsigned int)m_iNumSpokes; ++i)
		{
			double dis = (m_rpOutlineFeature->getGeometry()->at(i) - m_v3dEndLLH).length();
			mapDisToPos[dis] = i;
		}
		m_iDrawStart = mapDisToPos.begin()->second;
		m_iDrawCur = 0;
		m_bComputeCircleGrid = true;

		//m_bComputeViewLine = true;
	}


	void CRadialLineOfSightNode::StopCompute()
	{
		if(m_bComputeCircleGrid)
		{
			m_bComputeCircleGrid = false;
			m_iDrawCur = m_iNumSpokes;
			m_rpGridMultiGeometryGreen->getComponents().clear();
			m_rpGridFeatureNodeGreen->init();
			m_rpGridMultiGeometryRed->getComponents().clear();
			m_rpGridFeatureNodeRed->init();
		}

		if(m_bComputeViewLine)
		{
			m_bComputeViewLine = false;
			m_rpViewFeature->getGeometry()->clear();
			m_rpViewFeatureNode->init();
		}
	}

	void CRadialLineOfSightNode::UpdateGrid()
	{
		if (m_iDrawCur == m_iNumSpokes)
		{
			return;
		}		

		int numDraw = (m_iDrawStart + m_iDrawCur) % m_iNumSpokes;
		const osgEarth::SpatialReference* mapSRS = m_opMapNode->getMapSRS();
		const osgEarth::Terrain* terrain = m_opMapNode->getTerrain();
		VecColorToPos vecColorToPos;
		osg::Vec3d center(m_geoCenter.x(), m_geoCenter.y(), 0.0), firstWorld;

		osgEarth::GeoPoint firstPoint(mapSRS, center.x(), center.y(), m_dViewHeight,
            osgEarth::ALTMODE_RELATIVE);
		firstPoint.transform(mapSRS).toWorld(firstWorld, terrain);

		vecColorToPos.push_back(std::make_pair(true, osg::Vec2d(center.x(), center.y())));
		for (int i = 1; i <= m_iDetectNum; ++i)
		{
			double radio = i / (double)m_iDetectNum;
			osg::Vec3d v3dPoint = center + (m_rpOutlineFeature->getGeometry()->at(numDraw) - center) * i / m_iDetectNum;
            osgEarth::GeoPoint secondPoint(mapSRS, v3dPoint.x(), v3dPoint.y(), 0.1, osgEarth::ALTMODE_RELATIVE);

			//Computes the LOS and redraws the scene      
			osg::Vec3d secondWorld;
			if (!secondPoint.transform(mapSRS).toWorld(secondWorld, terrain))
			{
				return;
			}

			FeMeasure::CDPLineSegmentIntersector* lsi = new FeMeasure::CDPLineSegmentIntersector(firstWorld, secondWorld);
			osgUtil::IntersectionVisitor iv(lsi);
			m_opMapNode->accept(iv);

			if (lsi->containsIntersections())
			{
				vecColorToPos.push_back(std::make_pair(false, osg::Vec2d(v3dPoint.x(), v3dPoint.y())));
			}
			else
			{
				vecColorToPos.push_back(std::make_pair(true, osg::Vec2d(v3dPoint.x(), v3dPoint.y())));
			}
		}

		/// 将多个连续的相同颜色的线合并为一条线，提高效率(g00034 2016.10.11)
		osgEarth::Symbology::LineString * line = new osgEarth::Symbology::LineString();;
		for (int i = 0; i <= m_iDetectNum; ++i)
		{
			line->push_back(vecColorToPos[i].second.x(), vecColorToPos[i].second.y());	
			if (i<m_iDetectNum)
			{
				if (vecColorToPos[i].first != vecColorToPos[i+1].first)
				{			
					line->push_back(vecColorToPos[i+1].second.x(), vecColorToPos[i+1].second.y());
					if (vecColorToPos[i].first)
					{
						m_rpGridMultiGeometryGreen->add(line);
					}
					else
					{
						m_rpGridMultiGeometryRed->add(line);				
					}
					line = new osgEarth::Symbology::LineString();
				}
			}
			else
			{
				if (vecColorToPos[i].first)
				{
					m_rpGridMultiGeometryGreen->add(line);
				}
				else
				{
					m_rpGridMultiGeometryRed->add(line);
				}

			}
		}
		m_rpGridFeatureNodeGreen->init();
		m_rpGridFeatureNodeRed->init();

		m_vecColorToPos.push_back(vecColorToPos);

		++m_iDrawCur;

		/// FeatureNode中的geometry过多时，添加新的FeatureNode节点，提高渲染效率(g00034 2016.10.11)
		const int nMaxGeomNum = 60;
		if(m_rpGridMultiGeometryGreen->getComponents().size() > nMaxGeomNum)
		{
			RebuildGreenGeometry();
		}
		if(m_rpGridMultiGeometryRed->getComponents().size() > nMaxGeomNum)
		{
			RebuildRedGeometry();
		}
	}

	void CRadialLineOfSightNode::RebuildGreenGeometry()
	{
		m_rpGridMultiGeometryGreen = new osgEarth::Features::MultiGeometry();
		m_rpGridFeatureGreen = new osgEarth::Features::Feature(m_rpGridMultiGeometryGreen.get(), m_opMapNode->getMapSRS());   // 建立一个环几何体。
		m_rpGridFeatureGreen->geoInterp() = osgEarth::GEOINTERP_GREAT_CIRCLE;

		osgEarth::Symbology::AltitudeSymbol* pAlt = m_rpGridFeatureGreen->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
		pAlt->clamping() = pAlt->CLAMP_TO_TERRAIN;			// 绘制的线沿着地表
		pAlt->technique() = pAlt->TECHNIQUE_DRAPE;

		osgEarth::Symbology::LineSymbol* pLs = m_rpGridFeatureGreen->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
		pLs->stroke()->color() = osgEarth::Symbology::Color::Lime;
		pLs->stroke()->width() = 2.0f;
		pLs->tessellation() = 0;	
		osgEarth::Symbology::Style style;
		osgEarth::Features::GeometryCompilerOptions options;
		options.maxGranularity() = 180.0;
		m_rpGridFeatureNodeGreen = new osgEarth::Annotation::FeatureNode(m_opMapNode.get(), m_rpGridFeatureGreen.get(), style, options);
		m_rpGridFeatureNodeGreen->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		m_rpGridFeatureNodeGreen->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		m_rpGridFeatureNodeGreen->setClusterCulling(false);
		addChild(m_rpGridFeatureNodeGreen.get());
	}

	void CRadialLineOfSightNode::RebuildRedGeometry()
	{
		m_rpGridMultiGeometryRed = new osgEarth::Features::MultiGeometry();
		m_rpGridMultiGeometryRed->getComponents().clear();
		m_rpGridFeatureRed = new osgEarth::Features::Feature(m_rpGridMultiGeometryRed.get(), m_opMapNode->getMapSRS());   // 建立一个环几何体。
		m_rpGridFeatureRed->geoInterp() = osgEarth::GEOINTERP_GREAT_CIRCLE;

		osgEarth::Symbology::AltitudeSymbol* pAltRed = m_rpGridFeatureRed->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
		pAltRed->clamping() = pAltRed->CLAMP_TO_TERRAIN;			// 绘制的线沿着地表
		pAltRed->technique() = pAltRed->TECHNIQUE_DRAPE;

		osgEarth::Symbology::LineSymbol* pLsRed = m_rpGridFeatureRed->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
		pLsRed->stroke()->color() = osgEarth::Symbology::Color::Red;
		pLsRed->stroke()->width() = 2.0f;
		pLsRed->tessellation() = 0;		
		osgEarth::Symbology::Style style;
		osgEarth::Features::GeometryCompilerOptions options;
		options.maxGranularity() = 180.0;
		m_rpGridFeatureNodeRed = new osgEarth::Annotation::FeatureNode(m_opMapNode.get(), m_rpGridFeatureRed.get(), style, options);
		m_rpGridFeatureNodeRed->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		m_rpGridFeatureNodeRed->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		m_rpGridFeatureNodeRed->setClusterCulling(false);
		addChild(m_rpGridFeatureNodeRed.get());
	}

	void CRadialLineOfSightNode::CreateMouseLineElement()
	{
		m_rpMouseLineFeature= new osgEarth::Features::Feature(new osgEarth::Symbology::LineString(), m_opMapNode->getMapSRS());   // 建立一个环几何体。
		m_rpMouseLineFeature->geoInterp() = osgEarth::GEOINTERP_GREAT_CIRCLE;

		osgEarth::Symbology::AltitudeSymbol* pAlt = m_rpMouseLineFeature->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
		pAlt->clamping() = pAlt->CLAMP_TO_TERRAIN;			// 绘制的线沿着地表
		pAlt->technique() = pAlt->TECHNIQUE_SCENE;

		// define a style for the line
		osgEarth::Symbology::LineSymbol* pLs = m_rpMouseLineFeature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
		pLs->stroke()->color() = osgEarth::Symbology::Color::Cyan;
		pLs->stroke()->width() = 3.0f;
		pLs->tessellation() = 50;
		osgEarth::Symbology::Style style;
		osgEarth::Features::GeometryCompilerOptions options;
		options.maxGranularity() = 180.0;
		m_rpMouseLineFeatureNode= new osgEarth::Annotation::FeatureNode(m_opMapNode.get(), m_rpMouseLineFeature.get(),style, options);
		m_rpMouseLineFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		m_rpMouseLineFeatureNode->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		m_rpMouseLineFeatureNode->setClusterCulling(false);
		addChild(m_rpMouseLineFeatureNode.get());
	}

	void CRadialLineOfSightNode::UpdateMouseLine()
	{
		if (m_rpMouseLineFeatureNode.valid())
		{
			m_rpMouseLineFeature->getGeometry()->clear();
			m_rpMouseLineFeature->getGeometry()->push_back(m_geoCenter.vec3d().x(), m_geoCenter.vec3d().y());
			m_rpMouseLineFeature->getGeometry()->push_back(m_v3dEndLLH.x(), m_v3dEndLLH.y());
			m_rpMouseLineFeatureNode->init();
		}
	}

	void CRadialLineOfSightNode::ComputeCircleGrid()
	{
		if (m_bComputeCircleGrid && m_vecColorToPos.size() == m_iNumSpokes)
		{
			for (int i = 1; i <= m_iDetectNum; ++i)
			{
				osgEarth::Symbology::LineString * line = new osgEarth::Symbology::LineString();
				for (int j = 0; j < m_iNumSpokes; ++j)
				{
					ColorToPos curInfo1 = m_vecColorToPos[j][i];
					//osgEarth::Symbology::LineString * line = new osgEarth::Symbology::LineString();
					line->push_back(curInfo1.second.x(), curInfo1.second.y());

					int numSecond = j + 1;
					if (numSecond == m_iNumSpokes)  
						numSecond = 0; 

					ColorToPos curInfo2 = m_vecColorToPos[numSecond][i];

					if (numSecond != 0)
					{
						if (curInfo1.first!=curInfo2.first)
						{
							line->push_back(curInfo2.second.x(), curInfo2.second.y());
							if (curInfo1.first)
							{
								m_rpGridMultiGeometryGreen->add(line);
							}
							else
							{
								m_rpGridMultiGeometryRed->add(line);
							}
							line = new osgEarth::Symbology::LineString();
						}
					}
					else
					{
						line->push_back(curInfo2.second.x(), curInfo2.second.y());
						if (curInfo1.first)
						{
							m_rpGridMultiGeometryGreen->add(line);
						}
						else
						{
							m_rpGridMultiGeometryRed->add(line);
						}
					}
				}
			}

			m_rpGridFeatureNodeGreen->init();
			m_rpGridFeatureNodeRed->init();

			m_bComputeCircleGrid = false;
		}
	}

}

namespace FeMeasure
{
	CRadialLineCallBack::CRadialLineCallBack(CRadialLineOfSightNode* pSightNode) :
		m_opSightNode(pSightNode)
	{

	}

	CRadialLineCallBack::~CRadialLineCallBack()
	{

	}

	void CRadialLineCallBack::operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
		{
			if (node == m_opSightNode.get() && m_opSightNode.valid())
			{
				m_opSightNode->ComputeCircleGrid();
				m_opSightNode->UpdateGrid();
				m_opSightNode->ComputeViewLine();
			}
		}
	}

}
