#include <FeMeasure/BuildingsLinesMeasure.h>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/LineWidth>
#include <osgDB/ConvertUTF>

#include <osgEarth/DPLineSegmentIntersector>

#include <FeUtils/CoordConverter.h>
#include <osg/Depth>

namespace FeMeasure
{

	CBuildingsLinesResult::CBuildingsLinesResult( FeUtil::CRenderContext* pContext )
		: CMeasureResult(pContext)
	{
// 		m_rpMtRoot = new osg::MatrixTransform;
// 		addChild(m_rpMtRoot);
	}
	
	bool CBuildingsLinesResult::AddNewVisibleLine()
	{
		osgEarth::Symbology::Style style;
		osgEarth::Features::GeometryCompilerOptions options;
		options.maxGranularity() = 180.0;
		osgEarth::Annotation::FeatureNode* featureNode = new osgEarth::Annotation::FeatureNode(GetRenderContext()->GetMapNode(), 
			new osgEarth::Features::Feature(new osgEarth::Annotation::LineString(), 
			GetRenderContext()->GetMapNode()->getMapSRS()), style, options);

		featureNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		featureNode->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false), 
			osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		featureNode->setClusterCulling(false);

		addChild(featureNode);
		m_listLines.push_back(featureNode);

		return true;
	}

	bool CBuildingsLinesResult::CreateVisibleLine( unsigned int unIndex, osg::Vec3Array* pArrayVertex)
	{
		/// 如果目标点不存在，则添加
		if(!IsVisibleLineExist(unIndex))
		{
			AddNewVisibleLine();
		}

		if(IsVisibleLineExist(unIndex))
		{
			osgEarth::Annotation::FeatureNode* featureNode = m_listLines.at(unIndex).get();
			featureNode->getFeatures().clear();

// 			// 贴地线节点
// 		 	osg::ref_ptr<osgEarth::Annotation::FeatureNode> featureNodeSurface = new osgEarth::Annotation::FeatureNode(GetRenderContext()->GetMapNode(), 
// 				new osgEarth::Features::Feature(new osgEarth::Annotation::LineString(), 
// 				GetRenderContext()->GetMapNode()->getMapSRS()));
// 
// 			featureNodeSurface->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
// 			featureNodeSurface->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false), 
// 				osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
// 			featureNodeSurface->setClusterCulling(false);
// 			featureNodeSurface->getFeatures().clear();
// 
// 			// 面图元节点
// 			osg::ref_ptr<osgEarth::Annotation::FeatureNode> featureNodePolygon = new osgEarth::Annotation::FeatureNode(GetRenderContext()->GetMapNode(), 
// 				new osgEarth::Features::Feature(new osgEarth::Annotation::LineString(), 
// 				GetRenderContext()->GetMapNode()->getMapSRS()));
// 
// 			featureNodePolygon->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
// 			featureNodePolygon->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false), 
// 				osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
// 			featureNodePolygon->setClusterCulling(false);
// 			featureNodePolygon->getFeatures().clear();
// 
// 			featureNode->removeChildren(0, 10);
			
			for(int n = 1; n < pArrayVertex->size(); n+=2)
			{
				/// 空间直线
				osgEarth::Features::Feature* featureSpace = new osgEarth::Features::Feature(new osgEarth::Annotation::LineString(), 
					GetRenderContext()->GetMapNode()->getMapSRS());  

				featureSpace->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;

				// offset to mitigate Z fighting
				osgEarth::Symbology::RenderSymbol* pRender = featureSpace->style()->getOrCreate<osgEarth::Symbology::RenderSymbol>();
				pRender->depthOffset()->enabled() = true;
				pRender->depthOffset()->minBias() = 1000;

				osgEarth::Symbology::LineSymbol* pLs = featureSpace->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
				pLs->stroke()->width() = 3.0f;
				pLs->stroke()->widthUnits() = osgEarth::Symbology::Units::PIXELS;
				pLs->tessellation() = 20;

				featureSpace->getGeometry()->push_back(pArrayVertex->at(n-1));
				featureSpace->getGeometry()->push_back(pArrayVertex->at(n));


				/// 贴地线
// 				osgEarth::Features::Feature* featureSurface = new osgEarth::Features::Feature(new osgEarth::Annotation::LineString(), 
// 					GetRenderContext()->GetMapNode()->getMapSRS());  
// 
// 				featureSurface->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;
// 				
// 				osgEarth::Symbology::AltitudeSymbol* pAlt = featureSurface->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
// 				pAlt->clamping() = pAlt->CLAMP_TO_TERRAIN;			// 绘制的线沿着地表
// 				pAlt->technique() = pAlt->TECHNIQUE_DRAPE;
// 
// 				osgEarth::Symbology::LineSymbol* pLs1 = featureSurface->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
// 				pLs1->stroke()->width() = 1.5f;
// 				pLs1->stroke()->widthUnits() = osgEarth::Symbology::Units::PIXELS;
// 				pLs1->tessellation() = 150;
// 
// 				featureSurface->getGeometry()->push_back(pArrayVertex->at(n-1));
// 				featureSurface->getGeometry()->push_back(pArrayVertex->at(n));

				if(1 == n) // 设置通视线颜色，第一个线段为可视
				{
					pLs->stroke()->color() =  osgEarth::Symbology::Color(osg::Vec4d(0,1,0,1));// osgEarth::Symbology::Color::Green;
					//pLs1->stroke()->color() = osgEarth::Symbology::Color::Yellow;

					/// 通视区域，面标识
// 					osgEarth::Features::Feature* featurePolygon = new osgEarth::Features::Feature(new osgEarth::Annotation::LineString(), 
// 						GetRenderContext()->GetMapNode()->getMapSRS());  
// 
// 					featurePolygon->getGeometry()->push_back(pArrayVertex->at(n-1));
// 					featurePolygon->getGeometry()->push_back(pArrayVertex->at(n));
// 					featurePolygon->getGeometry()->push_back(osg::Vec3d(pArrayVertex->at(n).x(), pArrayVertex->at(n).y(),-10));
// 					featurePolygon->getGeometry()->push_back(osg::Vec3d(pArrayVertex->at(n-1).x(), pArrayVertex->at(n-1).y(),-10));
// 					featurePolygon->style()->getOrCreate<osgEarth::Symbology::PolygonSymbol>()->fill()->color().set(1,1,0,0.5);
// 					featurePolygon->style()->getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color().set(1,1,0,0.5);
// 				
// 					featureNodePolygon->getFeatures().push_back(featurePolygon);
// 					featureNodePolygon->setStyle(featurePolygon->style().get());
				}
				else
				{
					pLs->stroke()->color() = osgEarth::Symbology::Color::Red;
					//pLs1->stroke()->color() = osgEarth::Symbology::Color::Red;
				}

				featureNode->getFeatures().push_back(featureSpace);
				featureNode->setStyle(featureSpace->style().get());

//  				featureNodeSurface->getFeatures().push_back(featureSurface);
//  				featureNodeSurface->setStyle(featureSurface->style().get());
			}

			featureNode->init();

			//featureNode->addChild(featureNodeSurface);
			
// 			m_rpMtRoot->removeChildren(0,10);
// 			m_rpMtRoot->addChild(featureNodePolygon);
// 			featureNodePolygon->init();
		
			return true;
		}
		
		return false;
	}

	bool CBuildingsLinesResult::CreateTargetPointInfo( unsigned int unIndex, const osg::Vec3d& vecPosLLH, double distVisible, double distInvisible )
	{
		int nIndex = unIndex + 1; // 第一个点为观察点，目标点从第二个点开始

		if(!IsBillboardExist(nIndex))
		{
			FeMeasure::CMeasuringInfoDisplay *pBillBoard = new FeMeasure::CDisplayByBillBoard(GetRenderContext()->GetMapNode());

			addChild(pBillBoard->GetDisplayAsOsgNode());
			m_listBillBoardDisplay.push_back(pBillBoard);
		}

		if(!IsBillboardExist(nIndex)) return false;

		/// 设置信息
		if(IsBillboardPosNeedUpdate(nIndex, vecPosLLH))
		{
			FeMeasure::CMeasuringInfoDisplay *pBillBoard = m_listBillBoardDisplay.at(nIndex).get();

			pBillBoard->SetPosition(vecPosLLH);

			pBillBoard->SetOrCreateString(0, osgDB::convertStringFromCurrentCodePageToUTF8(std::string("角色: 被观察点")));

			char cTemp[100];
			std::sprintf(cTemp, "经度: %.3f度", vecPosLLH.x());
			pBillBoard->SetOrCreateString(1, osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));

			std::sprintf(cTemp, "纬度: %.3f度", vecPosLLH.y());
			pBillBoard->SetOrCreateString(2, osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));

			std::sprintf(cTemp, "高度: %.2f米", vecPosLLH.z());
			pBillBoard->SetOrCreateString(3, osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));

			std::sprintf(cTemp, "可视距离:   %.2f米", distVisible);
			pBillBoard->SetOrCreateString(4, osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));

			std::sprintf(cTemp, "不可视距离: %.2f米", distInvisible);
			pBillBoard->SetOrCreateString(5, osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
		}

		return true;
	}

	bool CBuildingsLinesResult::CreateLookPointInfo( unsigned int unIndex, const osg::Vec3d& vecPosLLH )
	{
		int nIndex = unIndex;

		if(!IsBillboardExist(nIndex))
		{
			FeMeasure::CMeasuringInfoDisplay *pBillBoard = new FeMeasure::CDisplayByBillBoard(
				GetRenderContext()->GetMapNode());

			addChild(pBillBoard->GetDisplayAsOsgNode());
			m_listBillBoardDisplay.push_back(pBillBoard);
		}

		if(!IsBillboardExist(nIndex)) return false;

		/// 设置信息
		if(IsBillboardPosNeedUpdate(nIndex, vecPosLLH))
		{
			FeMeasure::CMeasuringInfoDisplay *pBillBoard = m_listBillBoardDisplay.at(nIndex).get();

			pBillBoard->SetPosition(vecPosLLH);

			pBillBoard->SetOrCreateString(0, osgDB::convertStringFromCurrentCodePageToUTF8(std::string("角色: 观察点")));

			char cTemp[100];
			std::sprintf(cTemp, "经度: %.3f度", vecPosLLH.x());
			pBillBoard->SetOrCreateString(1, osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));

			std::sprintf(cTemp, "纬度: %.3f度", vecPosLLH.y());
			pBillBoard->SetOrCreateString(2, osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));

			std::sprintf(cTemp, "高度: %.2f米", vecPosLLH.z());
			pBillBoard->SetOrCreateString(3, osgDB::convertStringFromCurrentCodePageToUTF8(std::string(cTemp)));
		}

		return true;
	}

	osg::Vec3d CBuildingsLinesResult::GetFirstIntersectPoint(const osg::Vec3d& lookPoint, const osg::Vec3d& destPoint)
	{
		osgEarth::DPLineSegmentIntersector *picker = new osgEarth::DPLineSegmentIntersector(lookPoint, destPoint);
		osgUtil::IntersectionVisitor iv(picker);
		GetRenderContext()->GetMapNode()->accept(iv);

		if (picker->containsIntersections())
		{
			osgUtil::LineSegmentIntersector::Intersections intersections = picker->getIntersections();
			if(!intersections.empty())
			{
				return intersections.begin()->getWorldIntersectPoint();
			}
		}

		return destPoint;
	}

	void CBuildingsLinesResult::ProcessDestPoints(const osg::Vec3d& lookPointXYZ, const osg::Vec3dArray& vecDestPointsLLH)
	{
		osg::Vec3d vecLookLLH;
		FeUtil::XYZ2DegreeLLH(GetRenderContext(), lookPointXYZ, vecLookLLH);
		
		for(int n = 0; n < vecDestPointsLLH.size(); n++)
		{
			/// 不需要更新
			if(IsDestPointNotNeedUpdate(n, vecDestPointsLLH.at(n)))
			{
				continue;
			}

			/// 计算通视情况
			osg::Vec3d destPointXYZ, intersectPoint;
			FeUtil::DegreeLLH2XYZ(GetRenderContext(), vecDestPointsLLH.at(n), destPointXYZ);
			intersectPoint = GetFirstIntersectPoint(lookPointXYZ, destPointXYZ);
			
			osg::ref_ptr<osg::Vec3Array> pVertexArray = new osg::Vec3Array;
			double visibleDist, invisibleDist;
			
			if(intersectPoint == destPointXYZ) /// 无交点
			{
				pVertexArray->push_back(vecLookLLH);
				pVertexArray->push_back(vecDestPointsLLH.at(n));

				visibleDist = (destPointXYZ - lookPointXYZ).length();
				invisibleDist = 0;
			}
			else 
			{
				osg::Vec3d vecIntersectLLH;
				FeUtil::XYZ2DegreeLLH(GetRenderContext(), intersectPoint, vecIntersectLLH);

				pVertexArray->push_back(vecLookLLH);
				pVertexArray->push_back(vecIntersectLLH);
				pVertexArray->push_back(vecIntersectLLH);
				pVertexArray->push_back(vecDestPointsLLH.at(n));

				visibleDist = (intersectPoint - lookPointXYZ).length();
				invisibleDist = (destPointXYZ - intersectPoint).length();
			}
			
			/// 创建通视线段
			if(!CreateVisibleLine(n, pVertexArray))
			{
				return;
			}

			/// 添加标牌信息
			CreateTargetPointInfo(n, vecDestPointsLLH.at(n), visibleDist, invisibleDist);
		}
	}
		
	void CBuildingsLinesResult::ResetBillBoard( int numNeed )
	{
		if(m_listBillBoardDisplay.size() > numNeed)
		{
			BILLBOARD_LIST::iterator iter = m_listBillBoardDisplay.begin() + numNeed;
			while(iter != m_listBillBoardDisplay.end())
			{
				removeChild((iter++)->get()->GetDisplayAsOsgNode());
			}
			m_listBillBoardDisplay.erase(m_listBillBoardDisplay.begin() + numNeed, m_listBillBoardDisplay.end());
		}
	}

	void CBuildingsLinesResult::ResetVisibleLine( int numNeed )
	{
		if(m_listLines.size() > numNeed)
		{
			VISIBLE_LINE_LIST::iterator iter = m_listLines.begin() + numNeed;
			while(iter != m_listLines.end())
			{
				removeChild((iter++)->get());
			}
			m_listLines.erase(m_listLines.begin() + numNeed, m_listLines.end());
		}
	}

	bool CBuildingsLinesResult::CalculateImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		if (!GetRenderContext() || vecPointsLLH.empty())
		{
			return false;
		}

		/// 第一个点为观察点
		osg::Vec3d lookPtLLH = vecPointsLLH.front() + osg::Vec3d(0,0,2);
		ProcessLookPoint(lookPtLLH);
		
		/// 从第二个点开始均为目标点
		if(vecPointsLLH.size() > 1)
		{
			osg::ref_ptr<osg::Vec3dArray> destVertexes = new osg::Vec3dArray;
			destVertexes->assign(vecPointsLLH.begin()+1, vecPointsLLH.end());

			osg::Vec3d lookPointXYZ;
			FeUtil::DegreeLLH2XYZ(GetRenderContext(), lookPtLLH, lookPointXYZ);

			ProcessDestPoints(lookPointXYZ, *destVertexes);
		}
		
		/// 重置通视线段和标牌
		int billBoardNumNeed = vecPointsLLH.size();       // 应有标牌数
		int visibleLineNumNeed = vecPointsLLH.size() - 1; // 应有通视线数
		ResetBillBoard(billBoardNumNeed);
		ResetVisibleLine(visibleLineNumNeed);

		return true;
	}

}

namespace FeMeasure
{
	CBuildingsLinesMeasure::CBuildingsLinesMeasure( FeUtil::CRenderContext* pContext ) : CMeasure(pContext)
	{
		SetKey("BuildingsLinesMeasure");
		SetTitle("建筑群点对点通视分析");
		MeasureType() = EM_LineVisible;
	}
}


