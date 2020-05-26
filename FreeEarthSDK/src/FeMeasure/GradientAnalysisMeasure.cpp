#include <FeMeasure/GradientAnalysisMeasure.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/LineWidth>
#include <osg/Depth>
#include <osgDB/ConvertUTF>
#include <osgEarth/Units>

#include <osgEarth/DPLineSegmentIntersector>
#include <osgEarthAnnotation/FeatureNode>

#include <FeUtils/CoordConverter.h>
#include <FeAlg/CoordConverter.h>

#include <FeMeasure/BillBoardDisplay.h>
#include <FeMeasure/GradientMapEffect.h>
#include <FeMeasure/GradientBillBoardDisplay.h>

#include <osgEarthDrivers/engine_mp/TileNode>
#include <osgEarthDrivers/engine_mp/TerrainNode>



namespace FeMeasure
{
	/// 测试
	osg::Geometry* g_geomY = 0;
	osg::Geometry* g_geomZ = 0;
	osg::Geometry* g_geomLine = 0;
	void DrawLine(const osg::Vec3& vecPos, osg::Geometry* geom, osg::Matrix mxTrans)
	{
		osg::Vec3Array* vertexes = new osg::Vec3Array;
		vertexes->push_back(osg::Vec3(0, 0, 0) * mxTrans);
		vertexes->push_back((vecPos*10000000000) * mxTrans);
		geom->setVertexArray(vertexes);
	}

	osg::Geometry* AddDrawLine(FeUtil::CRenderContext* pRC, const osg::Vec4& vecColor)
	{
		osg::Geode* geode = new osg::Geode;
		osg::Geometry* geom = new osg::Geometry;
		geode->addDrawable(geom);

		osg::Vec3Array* vertexes = new osg::Vec3Array;
		vertexes->push_back(osg::Vec3(0, 0, 0));
		vertexes->push_back(osg::Vec3(0, 0, 0));
		geom->setVertexArray(vertexes);

		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(vecColor);
		geom->setColorArray(colors, osg::Array::BIND_OVERALL);

		geom->addPrimitiveSet(new osg::DrawArrays(osg::DrawArrays::LINES, 0, vertexes->size()));

		geom->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		geom->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
		geom->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		pRC->GetRoot()->addChild(geode);

		return geom;
	}

	CGradientAnalysisResult::CGradientAnalysisResult( FeUtil::CRenderContext* pContext ) 
		: CMeasureResult(pContext)
		, m_bInstallEffect(false)
	{
		if(pContext)
		{
			m_opMapNode = pContext->GetMapNode();
		}
		
		CreateElement();
	}

	CGradientAnalysisResult::~CGradientAnalysisResult()
	{
		if (m_rpGradientMap.valid() && m_opMapNode.valid())
		{
			m_rpGradientMap->onUninstall(m_opMapNode->getTerrainEngine());
		}
	}

	bool CGradientAnalysisResult::CalculateImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		/// 两个点以上才进行处理
		if(vecPointsLLH.size() > 1)
		{
			/// 只使用首尾两个点进行计算
			osg::Vec3d firstPt = vecPointsLLH.front();
			osg::Vec3d curPt = vecPointsLLH.back();
			
			m_opLinePoints->clear();
			m_opLinePoints->push_back(firstPt.x(), firstPt.y());
			m_opLinePoints->push_back(curPt.x(), firstPt.y());
			m_opLinePoints->push_back(curPt.x(), curPt.y());
			m_opLinePoints->push_back(firstPt.x(), curPt.y());
			//m_opFeatureNode->init();
			
			DealPositionInformation();

			if (!m_bInstallEffect&& m_rpGradientMap.valid())
			{
				m_rpGradientMap->onInstall(m_opMapNode->getTerrainEngine());
				addChild(m_rpGradientBillBoard);
				m_rpGradientBillBoard->SetVisible(false);
				//OSG_NOTICE << "install : " << std::endl;
				m_bInstallEffect = true;
			}

			return true;
		}

		return false;
	}

	void CGradientAnalysisResult::CreateElement()
	{
		m_opLinePoints = new osgEarth::Symbology::Ring();
		m_opFeature = new osgEarth::Features::Feature(m_opLinePoints.get(), m_opMapNode->getMapSRS());
		m_opFeature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;

		osgEarth::Style lineStyle;
		lineStyle.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping()
			= osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
		lineStyle.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique()
			= osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_SCENE;
		lineStyle.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->widthUnits()
			= osgEarth::Symbology::Units::PIXELS;
		lineStyle.getOrCreate<osgEarth::Symbology::LineSymbol>()->tessellationSize() = osgEarth::Distance(1.0e+4);
		lineStyle.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = osgEarth::Symbology::Color::Yellow;
		lineStyle.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = 2.0;
		lineStyle.getOrCreate<osgEarth::Symbology::LineSymbol>()->tessellation() = 150;
		//lineStyle.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->stipple() = 255;  // 设置为虚线

		osgEarth::Features::GeometryCompilerOptions options;
		options.maxGranularity() = 180.0;
		m_opFeatureNode = new osgEarth::Annotation::FeatureNode(m_opMapNode.get(), m_opFeature.get(), lineStyle, options);
		m_opFeatureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		m_opFeatureNode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

		//m_opFeatureNode->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		m_opFeatureNode->setClusterCulling(false);

		const osgEarth::Config& normalMapConf = m_opMapNode->externalConfig().child("normal_map");
		if (!normalMapConf.empty())
		{
			m_rpGradientMap = new FeMeasure::CGradientMapEffect(m_opMapNode.get());
			m_rpGradientMap->SetNormalMapConfig(normalMapConf, m_opMapNode->getMap());
			this->addCullCallback(new FeMeasure::CGradientArrowCullCallback(m_rpGradientMap.get()));
		}

		osgEarth::GeoPoint point(m_opMapNode->getMapSRS(), 0.0, 0.0, 0.0, osgEarth::ALTMODE_ABSOLUTE);
		m_rpGradientBillBoard = new FeMeasure::CGradientBillBoardDisplay(m_opMapNode.get(), point);
		m_rpGradientBillBoard->setNodeMask(false);
		//addChild(m_opFeatureNode.get());
	}

	void CGradientAnalysisResult::ShowSlopeAnalysisResult(osg::Vec3d llh)
	{
		if (m_rpGradientMap.valid() && m_rpGradientMap->IsOnInstall())
		{
			osg::Vec3f lonRange, latRange;
			m_rpGradientMap->GetLonLatRange(lonRange, latRange);
			if (llh.x() > lonRange.x() && llh.x() < lonRange.y() && llh.y() > latRange.x() && llh.y() < latRange.y())
			{
				osgEarth::GeoPoint point(m_opMapNode->getMapSRS(), llh.x(), llh.y(), 0.0, osgEarth::ALTMODE_RELATIVE);
				m_rpGradientBillBoard->setPosition(point);
				if (!m_rpGradientBillBoard->IsVisible())
				{
					m_rpGradientBillBoard->SetVisible(true);
				}

				double dHeight;
				m_opMapNode->getTerrain()->getHeight(m_opMapNode->getMapSRS(), llh.x(), llh.y(), &dHeight);

				char cTemp[100];
				std::sprintf(cTemp, "经度: %.6f度", llh.x());
				m_rpGradientBillBoard->SetText(1,
					osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
				std::sprintf(cTemp, "纬度: %.6f度", llh.y());
				m_rpGradientBillBoard->SetText(2,
					osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
				std::sprintf(cTemp, "高度: %.4f度", dHeight);
				m_rpGradientBillBoard->SetText(3,
					osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));

				//根据经纬度求出坡度信息
				//double dAngle = GetSlopeByLL(llh.x(), llh.y(), 500);
				osg::Vec2d gradientInfo;
				m_rpGradientMap->CalculateGradientInfo(osg::Vec2d(llh.x(), llh.y()), gradientInfo);

				std::sprintf(cTemp, "坡度: %.5f度", gradientInfo.x());
				m_rpGradientBillBoard->SetText(4,
					osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
				std::sprintf(cTemp, "坡向: %.4f度", gradientInfo.y());
				m_rpGradientBillBoard->SetText(5,
					osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
			}
			else {
				m_rpGradientBillBoard->SetVisible(false);
			}

		}
	}

	void CGradientAnalysisResult::DealPositionInformation()
	{
		if (m_opLinePoints.valid() && m_rpGradientMap.valid() && m_opLinePoints->size() >= 4)
		{
			osg::Vec3d firstPoint = m_opLinePoints->at(0);
			osg::Vec3d secondPoint = m_opLinePoints->at(2);
			osg::Vec3f longitudeRange, latitudeRange;
			bool bSameArea = ((firstPoint.x() * secondPoint.x()) > 0);

			// 经度横跨东西经时需要特殊处理
			if(!bSameArea && (abs(firstPoint.x())+abs(secondPoint.x()) > 180)) 
			{
				if (firstPoint.x() > secondPoint.x())
				{
					longitudeRange = osg::Vec3f(firstPoint.x(), secondPoint.x(), -1.0);
				}
				else {
					longitudeRange = osg::Vec3f(secondPoint.x(), firstPoint.x(), -1.0);
				}
			}
			else
			{
				if (firstPoint.x() < secondPoint.x())
				{
					longitudeRange = osg::Vec3f(firstPoint.x(), secondPoint.x(), 1.0);
				}
				else {
					longitudeRange = osg::Vec3f(secondPoint.x(), firstPoint.x(), 1.0);
				}
			}
			
			if (firstPoint.y() < secondPoint.y())
			{
				latitudeRange = osg::Vec3f(firstPoint.y(), secondPoint.y(), 1.0);
			}
			else {
				latitudeRange = osg::Vec3f(secondPoint.y(), firstPoint.y(), 1.0);
			}
			m_rpGradientMap->SetLonLatRange(longitudeRange, latitudeRange);
		}
	}


	class TileNodeVisitor : public osg::NodeVisitor
	{
	private:
		osg::Vec2d  m_vecLLDegree;
		osgEarth::Drivers::MPTerrainEngine::TileNode* m_pCurTileNode;
		osg::Vec4   m_vecNorm;
	public:
		TileNodeVisitor()
			: NodeVisitor(osg::NodeVisitor::NODE_VISITOR,osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
		{
		}

		osgEarth::Drivers::MPTerrainEngine::TileNode* GetTileNode(const osg::Vec2d& vecLLDegree, osg::Node* pNode)
		{
			m_pCurTileNode = 0;

			if(pNode)
			{
				m_vecLLDegree = vecLLDegree;
				pNode->accept(*this);
			}

			return m_pCurTileNode;
		}

		osg::Vec2 GetPosOnTile(osg::Vec2 lonRange, osg::Vec2 latRange, osg::Vec2 llPos)
		{
			double lonLen = lonRange.y() - lonRange.x();
			double latLen = latRange.y() = latRange.x();

			osg::Vec2 texPos;
			texPos.x() = (llPos.x() - lonRange.x()) * (lonLen);
			texPos.y() = (llPos.y() - latRange.x()) * (latLen);

			std::cout << "Texpos:" << texPos.x() << ", " << texPos.y() << std::endl;

			return texPos;
		}

		virtual void apply(osg::MatrixTransform& node)
		{
			osgEarth::Drivers::MPTerrainEngine::TileNode* tNode = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::TileNode*>(&node);
			if(tNode)
			{
				const osgEarth::Drivers::MPTerrainEngine::TileModel* tm = tNode->getTileModel();
				if(tm)
				{
					osgEarth::GeoExtent extent = tm->_tileKey.getExtent();
					osgEarth::GeoPoint lowerLeft(extent.getSRS(), extent.xMin(), extent.yMin(), 0.0, osgEarth::ALTMODE_ABSOLUTE);
					osgEarth::GeoPoint upperRight(extent.getSRS(), extent.xMax(), extent.yMax(), 0.0, osgEarth::ALTMODE_ABSOLUTE);

					if(extent.contains(m_vecLLDegree.x(), m_vecLLDegree.y()))
					{
// 						std::cout << "Lon.........." << lowerLeft.x() << " ----" << upperRight.x() << std::endl;
// 						std::cout << "Lat.........." << lowerLeft.y() << " ----" << upperRight.y() << std::endl;
						//return;

						m_pCurTileNode = tNode;
					}
				}
			}

			traverse(node);
		}
	};


	bool CGradientAnalysisResult::GetGradientInfo( const osg::Vec3d& posLLH, float& fSlope, float& fAspect )
	{
		if (m_rpGradientMap.valid() && m_rpGradientMap->IsOnInstall())
		{
			osg::Vec3f lonRange, latRange;
			m_rpGradientMap->GetLonLatRange(lonRange, latRange);
			if (posLLH.x() > lonRange.x() && posLLH.x() < lonRange.y() && posLLH.y() > latRange.x() && posLLH.y() < latRange.y())
			{
// 				osg::Vec2d gradientInfo;
// 				m_rpGradientMap->CalculateGradientInfo(osg::Vec2d(posLLH.x(), posLLH.y()), gradientInfo);
// 				fSlope = gradientInfo.x();
// 				fAspect = gradientInfo.y();


				/**
				先获取当前点的法线，在进行坡度坡向计算。
				坡度 = 法线与Z轴的夹角
				坡向 = 法线在XY平面上的投影与Y轴的夹角
				*/

				/// 根据当前点所在瓦片获取法线
				if(0) // 目前无法从瓦片中获取法线信息，法线贴图为空！
				{
					/// 获取当前点所在瓦片
					osgEarth::Drivers::MPTerrainEngine::TerrainNode* tNode = osgEarth::findRelativeNodeOfType<
						osgEarth::Drivers::MPTerrainEngine::TerrainNode>( GetRenderContext()->GetView()->getCamera() );
					if(tNode && tNode->getNumChildren() > 0 && tNode->getChild(0))
					{
						TileNodeVisitor nv;
						osgEarth::Drivers::MPTerrainEngine::TileNode* pTileNode = nv.GetTileNode(osg::Vec2d(posLLH.x(), posLLH.y()), tNode->getChild(0));
						if(!pTileNode) return false;

						/// 获取法线图层
						osgEarth::ImageLayer* layer = m_rpGradientMap->GetImgLayer();
						const osgEarth::Drivers::MPTerrainEngine::TileModel* pTm = pTileNode->getTileModel();
						if(layer && pTm)
						{
							/// 从法线图层中找到瓦片对应的纹理
							osg::Texture2D* pTileTex = 0;

							std::map<osgEarth::Drivers::MPTerrainEngine::UID, osgEarth::Drivers::MPTerrainEngine::TileModel::ColorData>::const_iterator itr 
								= pTm->_colorData.begin();
							for(; itr != pTm->_colorData.end(); itr++)
							{
								if( itr->second._layer.get() == layer)
								{
									pTileTex = dynamic_cast<osg::Texture2D*>(itr->second._texture.get());
									break;
								}
							}

							if(pTileTex && pTileTex->getImage())
							{
								osgEarth::GeoExtent extent = pTileNode->getTileModel()->_tileKey.getExtent();
								osg::Vec4 vecNom = pTileTex->getImage()->getColor( nv.GetPosOnTile(osg::Vec2(extent.xMin(), extent.xMax()), 
									osg::Vec2(extent.yMin(), extent.yMax()), osg::Vec2d(posLLH.x(), posLLH.y())) );

								std::cout << "norm--------------> " << vecNom.x() << "," << vecNom.y() << "," << vecNom.z() << "," << vecNom.w() << "," << std::endl;
							}
							else
							{
								osgEarth::GeoImage image = layer->createImage(pTm->_tileKey);	
								if(image.getImage())
								{
									/// 获取当前位置对应的法线纹理坐标
									const osgEarth::GeoExtent &extent = pTm->_tileKey.getExtent();
									osg::Vec2 texPos;
									texPos.x() = (posLLH.x() - extent.xMin()) / (extent.xMax() - extent.xMin());
									texPos.y() = (posLLH.y() - extent.yMin()) / (extent.yMax() - extent.yMin());

									//std::cout << "texPos>>>>> " << texPos.x() << ", " << texPos.y() << std::endl;

									osg::Vec4 vecNom = image.getImage()->getColor( texPos );

									/// 测试，绘制当前瓦片
// 									osg::ref_ptr<osg::Vec3dArray> vertexs = new osg::Vec3dArray;
// 									vertexs->push_back(osg::Vec3(extent.xMin(), extent.yMin(), 0));
// 									vertexs->push_back(osg::Vec3(extent.xMax(), extent.yMax(), 0));
// 									CalculateImplement(*vertexs);
								}
							}
						}
					}
				}


				/// 使用当前点所在三角面求法线
				if(1)
				{
					osg::Vec3d vecNom, xyz;
					FeUtil::DegreeLLH2XYZ(GetRenderContext(), posLLH+osg::Vec3(0,0,2), xyz);
					osg::ref_ptr< osgEarth::DPLineSegmentIntersector > picker = new osgEarth::DPLineSegmentIntersector(
						osgUtil::Intersector::MODEL, osg::Vec3d(0, 0, 0), xyz);

					// Limit it to one intersection, we only care about the first
					picker->setIntersectionLimit( osgUtil::Intersector::LIMIT_NEAREST );

					osgUtil::IntersectionVisitor iv(picker.get());
					GetRenderContext()->GetMapNode()->accept(iv);

					if (picker->containsIntersections())
					{        
						osgUtil::LineSegmentIntersector::Intersections  intersections = picker->getIntersections();
						osgUtil::LineSegmentIntersector::Intersections::iterator iter = intersections.begin();

						osg::Geometry* geometry = iter->drawable->asGeometry();
						if (geometry)
						{
							osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
							//osg::Matrix matrix = *iter->matrix.get();

							/// 根据三角面的顶点计算法线
							osg::Vec3d v1 = vertices->at(iter->indexList.at(0));
							osg::Vec3d v2 = vertices->at(iter->indexList.at(1));
							osg::Vec3d v3 = vertices->at(iter->indexList.at(2));

							vecNom = (v2-v1)^(v3-v2);
						}
					}
					else
					{
						//std::cout << "errrr: No intersect" << std::endl;
						return false;
					}

					/// 根据法线求坡度坡向
					vecNom.normalize();
					osg::Vec3 axisZ(osg::Z_AXIS), axisY(osg::Y_AXIS);

					double dSlopeCos = std::acos(vecNom * axisZ);
					fSlope = osg::RadiansToDegrees(dSlopeCos);
				
					osg::Vec3d normOnXY = osg::Vec3d(vecNom.x(), vecNom.y(), 0);
					normOnXY.normalize();
					double dAspect = std::acos( normOnXY * axisY);
					fAspect = osg::RadiansToDegrees(dAspect);

					/// 绘制法线，测试
// 					if(!g_geomLine)
// 					{
// 						g_geomY = AddDrawLine(GetRenderContext(), osg::Vec4(1, 0, 0, 1));
// 						g_geomZ = AddDrawLine(GetRenderContext(), osg::Vec4(0, 1, 0, 1));
// 						g_geomLine = AddDrawLine(GetRenderContext(), osg::Vec4(0, 0, 1, 1));
// 					}
// 					osg::Matrix mxTrans;
// 					FeUtil::DegreeLLH2Matrix(GetRenderContext(), posLLH, mxTrans);
// 					DrawLine(osg::Vec3(vecNom.x(), vecNom.y(), vecNom.z()), g_geomLine, mxTrans);
// 					DrawLine(osg::Vec3(axisZ.x(), axisZ.y(), axisZ.z()), g_geomZ, mxTrans);
// 					DrawLine(osg::Vec3(axisY.x(), axisY.y(), axisY.z()), g_geomY, mxTrans);
				}

				return true;
			}
		}

		return false;
	}

}

namespace FeMeasure
{
	CGradientAnalysisMeasure::CGradientAnalysisMeasure( FeUtil::CRenderContext* pContext ) 
		: CMeasure(pContext)
	{
		SetKey("GradientAnalysisMeasure");
		SetTitle("坡度坡向分析");
		MeasureType() = EM_GradientAnalysis;

		m_rpMeasureInfoDisplay = new CDisplayByBillBoard(GetRenderContext()->GetMapNode());
		addChild(m_rpMeasureInfoDisplay->GetDisplayAsOsgNode());
		m_rpMeasureInfoDisplay->SetVisible(false);

		this->setNumChildrenRequiringEventTraversal(1);
	}

	void CGradientAnalysisMeasure::AddCtrlPointImplement( const osg::Vec3d& vecPosLLH )
	{
		if(Vertexes().size() == 2) 
			Vertexes().back() = vecPosLLH;
		else
			Vertexes().push_back(vecPosLLH);
	}

	void CGradientAnalysisMeasure::SetCtrlPointsImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		if(!vecPointsLLH.empty())
		{
			Vertexes().clear();
			Vertexes().push_back(vecPointsLLH.front());
			Vertexes().push_back(vecPointsLLH.back());
		}
	}

	void CGradientAnalysisMeasure::ShowGradientInfo( const osg::Vec3d& vecPosLLH )
	{
		float fSlope, fAspect;

		for(unsigned int uIndex = 0; uIndex < GetMeasureResultCount(); uIndex++)
		{
			if(static_cast<CGradientAnalysisResult*>(GetMeasureResult(uIndex))->GetGradientInfo(vecPosLLH, fSlope, fAspect))
			{
				m_rpMeasureInfoDisplay->SetVisible(true);
				m_rpMeasureInfoDisplay->SetPosition(vecPosLLH);

				double dHeight;
				GetRenderContext()->GetMapNode()->getTerrain()->getHeight(
					GetRenderContext()->GetMapNode()->getMapSRS(), vecPosLLH.x(), vecPosLLH.y(), &dHeight);

				char cTemp[100];
				std::sprintf(cTemp, "经度: %.6f度", vecPosLLH.x());
				m_rpMeasureInfoDisplay->SetOrCreateString(0,
					osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
				std::sprintf(cTemp, "纬度: %.6f度", vecPosLLH.y());
				m_rpMeasureInfoDisplay->SetOrCreateString(1,
					osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
				std::sprintf(cTemp, "高度: %.4f度", dHeight);
				m_rpMeasureInfoDisplay->SetOrCreateString(2,
					osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
				std::sprintf(cTemp, "坡度: %.5f度", fSlope);
				m_rpMeasureInfoDisplay->SetOrCreateString(3,
					osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
				std::sprintf(cTemp, "坡向: %.4f度", fAspect);
				m_rpMeasureInfoDisplay->SetOrCreateString(4,
					osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));

				return;
			}
		}

		m_rpMeasureInfoDisplay->SetVisible(false);
	}

	void CGradientAnalysisMeasure::traverse( osg::NodeVisitor& nv )
	{
		if( !IsMeasuring() 
			&& (nv.getVisitorType() == osg::NodeVisitor::EVENT_VISITOR) 
			&& dynamic_cast<osgGA::EventVisitor*>(&nv) )
		{
			osgGA::EventVisitor* ev = static_cast<osgGA::EventVisitor*>(&nv);

			for(osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
				itr != ev->getEvents().end(); ++itr)
			{
				osgGA::GUIEventAdapter* ea = dynamic_cast<osgGA::GUIEventAdapter*>(itr->get());

				if(ea && (ea->getEventType() == osgGA::GUIEventAdapter::MOVE))
				{
					osg::Vec3d vecLLH(0.0, 0.0, 0.0);
					if(FeUtil::ScreenXY2DegreeLLH(GetRenderContext(), ea->getX(), ea->getY(), vecLLH))
					{
						ShowGradientInfo(vecLLH);
						break;
					}
				}
			}
		}

		CMeasure::traverse(nv);
	}

}

