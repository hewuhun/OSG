#include <FeMeasure/ContourMeasure.h>

#include <osgEarthSymbology/Geometry>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/RenderSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthSymbology/TextSymbol>

#include <FeUtils/MathUtil.h>
#include <FeUtils/CoordConverter.h>
#include <osgEarthAnnotation/LabelNode>
#include <osgEarth/GeoData>
#include <osgEarthFeatures/Feature>
#include <osgEarthDrivers/engine_mp/TerrainNode>
#include "osgText/Text"
#include "osg/Depth"
#include <osgEarth/Decluttering>
namespace FeMeasure
{
	class ContourTileNodeVisitor : public osg::NodeVisitor
	{
	private:
		osg::Vec2d  m_vecLLDegree;
		osgEarth::Drivers::MPTerrainEngine::TileNode* m_pCurTileNode;
		osg::Vec4   m_vecNorm;
		bool        bGetState;
		int m_nLod;
		int m_nX;
		int m_nY;
	public:
		ContourTileNodeVisitor()
			: NodeVisitor(osg::NodeVisitor::NODE_VISITOR,osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
			,bGetState(false)
			,m_pCurTileNode(nullptr)
			,m_nLod(0)
			,m_nX(0)
			,m_nY(0)
		{
		}

		osgEarth::Drivers::MPTerrainEngine::TileNode* GetTileNode(const osg::Vec2d& vecLLDegree, osg::Node* pNode)
		{
			m_pCurTileNode = nullptr;
			if(pNode)
			{
				bGetState = true;
				m_vecLLDegree = vecLLDegree;
				pNode->accept(*this);
			}
			return m_pCurTileNode;
		}

		osgEarth::Drivers::MPTerrainEngine::TileNode* GetTileNode(int nLod,int nX,int nY, osg::Node* pNode)
		{
			m_pCurTileNode = nullptr;
			if(pNode)
			{
				bGetState = false;
				m_nLod = nLod;
				m_nX = nX;
				m_nY = nY;
				pNode->accept(*this);
			}
			return m_pCurTileNode;
		}

		virtual void apply(osg::MatrixTransform& node)
		{
			osgEarth::Drivers::MPTerrainEngine::TileNode* tNode = dynamic_cast<osgEarth::Drivers::MPTerrainEngine::TileNode*>(&node);
			if(tNode)
			{
				osgEarth::TileKey tileKey = tNode->getKey();
				if(bGetState)
				{
					osgEarth::GeoExtent extent = tileKey.getExtent();
					if(extent.contains(m_vecLLDegree.x(), m_vecLLDegree.y()))
					{
						m_pCurTileNode = tNode;
					}
				}
				else
				{
					int nLod = tileKey.getLOD();
					int nX = tileKey.getTileX();
					int nY = tileKey.getTileY();
					if (m_nLod==nLod&&m_nX==nX&&m_nY==nY)
					{
						m_pCurTileNode = tNode;
					}
				}		
			}
			traverse(node);
		}
	};

	CContourResult::CContourResult( FeUtil::CRenderContext* pContext ) 
		: CMeasureResult(pContext)
		,m_nInterval(100)
		,m_nTextInterval(200)
		,m_nColumns(500)
		,m_nRows(500)
		,m_rpBorderNode(NULL)
		,m_rpContourNode(NULL)
		,m_rpTextGroup(NULL)
		,m_bUpdateTile(false)
		,m_bRunning(false)
	{
		m_pMapNode = GetRenderContext()->GetMapNode();	
		m_pMap = m_pMapNode->getMap();
		m_pSrs = m_pMap->getSRS();
		m_pMap->getElevationLayers(m_Elv);
		m_rpTextGroup = new osg::Group;
		addChild(m_rpTextGroup.get());
		m_LabelStyle.getOrCreate<osgEarth::TextSymbol>()->alignment() = osgEarth::TextSymbol::ALIGN_CENTER_CENTER;
		m_LabelStyle.getOrCreate<osgEarth::TextSymbol>()->fill()->color() = osgEarth::Color::White;
		//m_LabelStyle.getOrCreate<osgEarth::TextSymbol>()->halo()->color() = Color::Black;
		m_LabelStyle.getOrCreate<osgEarth::TextSymbol>()->font() = "simhei.ttf";
		m_LabelStyle.getOrCreate<osgEarth::TextSymbol>()->size() = 17.0;
		m_LabelStyle.getOrCreate<osgEarth::TextSymbol>()->encoding() = osgEarth::Symbology::TextSymbol::ENCODING_UTF8;
		m_LabelStyle.getOrCreate<osgEarth::TextSymbol>()->declutter() = true;
		m_LabelStyle.getOrCreate<osgEarth::TextSymbol>()->occlusionCull() = true;


		m_Linestyle.getOrCreate<osgEarth::LineSymbol>()->stroke()->color() = osgEarth::Symbology::Color(0.0,1.0,0.0,0.8);
		m_Linestyle.getOrCreate<osgEarth::LineSymbol>()->stroke()->width() = 1.0;
		//m_Linestyle.getOrCreate<LineSymbol>()->tessellation() = 100;
		m_Linestyle.getOrCreate<osgEarth::AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN;
		//m_Linestyle.getOrCreate<AltitudeSymbol>()->verticalOffset() = RELATIVE_ALTITUDE;
		m_Linestyle.getOrCreate<osgEarth::AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_GPU;
		getOrCreateStateSet()->setRenderBinDetails(100, "RenderBin");

	}

	CContourResult::~CContourResult()
	{
		for (int i=0; i<m_vecPointData.size(); i++)
		{
			delete m_vecPointData.at(i);
		}
	}

	void CContourResult::run()
	{
		m_bRunning = true;
		if (ComputePointData())
		{
			if (m_bUpdateTile)
			{
				if (ComputevecTileNode())
				{
					if(ComputePointDataLevel())
					{
						CreatContourNode();
					}		
				}
			}
			else
			{
				if(ComputePointDataLevel())
				{
					CreatContourNode();
				}
			}
		}
	}

	void CContourResult::UpdateContourNode(bool bUpdateTile)
	{
		nMinContourValue = 100000;
		nMaxContourValue = 0;

		m_bUpdateTile = bUpdateTile;
		m_bRunning = true;
		if (ComputePointData())
		{
			if (m_bUpdateTile)
			{
				if (ComputevecTileNode())
				{
					if(ComputePointDataLevel())
					{
						CreatContourNode();
					}		
				}
			}
			else
			{
				if(ComputePointDataLevel())
				{
					CreatContourNode();
				}
			}
		}
		//run();
	}

	bool CContourResult::CalculateImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		m_vecStart = vecPointsLLH.front();
		m_vecEnd = vecPointsLLH.back();
		if (m_rpBorderNode.valid())
		{
			removeChild(m_rpBorderNode.get());
			m_rpBorderNode.release();
		}
		m_dMinX = m_vecStart.x()<m_vecEnd.x()?m_vecStart.x():m_vecEnd.x();
		m_dMaxX = m_vecStart.x()>m_vecEnd.x()?m_vecStart.x():m_vecEnd.x();
		m_dMinY = m_vecStart.y()<m_vecEnd.y()?m_vecStart.y():m_vecEnd.y();
		m_dMaxY = m_vecStart.y()>m_vecEnd.y()?m_vecStart.y():m_vecEnd.y();
		m_vecLU = osg::Vec3d(m_dMinX,m_dMaxY,0.0);
		m_vecRU = osg::Vec3d(m_dMaxX,m_dMaxY,0.0);
		m_vecRD = osg::Vec3d(m_dMaxX,m_dMinY,0.0);
		m_vecLD = osg::Vec3d(m_dMinX,m_dMinY,0.0);
		osgEarth::Features::FeatureList m_featureList;
		osg::ref_ptr<osgEarth::Symbology::Ring> rpLine = new osgEarth::Symbology::Ring;
		rpLine->push_back(m_vecLU);
		rpLine->push_back(m_vecRU);
		rpLine->push_back(m_vecRD);
		rpLine->push_back(m_vecLD);
		osg::ref_ptr<osgEarth::Features::Feature> rpLineFeature = new osgEarth::Features::Feature(rpLine.get(), m_pSrs, osgEarth::Symbology::Style());
		rpLineFeature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;
		m_featureList.push_back(rpLineFeature);
		osgEarth::Style linestyle;
		linestyle.getOrCreate<osgEarth::LineSymbol>()->stroke()->color() = osgEarth::Symbology::Color(1.0,1.0,0.0,0.8);
		linestyle.getOrCreate<osgEarth::LineSymbol>()->stroke()->width() = 2.0;
		linestyle.getOrCreate<osgEarth::LineSymbol>()->tessellation() = 100;
		linestyle.getOrCreate<osgEarth::AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN;
		linestyle.getOrCreate<osgEarth::AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_GPU;
		m_rpBorderNode = new osgEarth::Annotation::FeatureNode(m_pMapNode, m_featureList, linestyle);
		m_rpBorderNode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		m_rpBorderNode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		m_rpBorderNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		addChild(m_rpBorderNode.get());
		return true;
	}

	bool CContourResult::FinalCalculateImplement()
	{
		UpdateContourNode(true);
		return true;
	}

	bool CContourResult::ComputePointData()
	{
		if (m_vecPointData.size()>0)
		{
			for (int i=0; i<m_vecPointData.size(); i++)
			{
				delete m_vecPointData.at(i);
			}
			m_vecPointData.clear();
		}	
		for (int nRow=0;nRow<m_nRows;++nRow)
		{
			for (int nColumn=0;nColumn<m_nColumns;++nColumn)
			{
				if (m_bRunning)
				{
					auto x1 = m_vecLU.x()+(m_vecRU.x()-m_vecLU.x())*nColumn/m_nColumns;
					auto x2 = m_vecLD.x()+(m_vecRD.x()-m_vecLD.x())*nColumn/m_nColumns;
					auto y1 = m_vecLU.y()+(m_vecRU.y()-m_vecLU.y())*nColumn/m_nColumns;
					auto y2 = m_vecLD.y()+(m_vecRD.y()-m_vecLD.y())*nColumn/m_nColumns;
					osg::Vec3d vecLLH(x1+(x2-x1)*nRow/m_nRows,y1+(y2-y1)*nRow/m_nRows,0.0);
					SPointData* pData = new SPointData;
					pData->vecLLH = vecLLH;
					pData->nColumn = nColumn;
					pData->nRow = nRow;
					m_vecPointData.push_back(pData);	
				}
				else
					return false;
			}
		}
		return true;
	}

	bool CContourResult::ComputevecTileNode()
	{
		if (m_vecTileNode.size()>0)
		{
			m_vecTileNode.clear();
		}
		int nTileLod = 14;
		osg::Vec2 vecCenter(GetRenderContext()->GetView()->getCamera()->getViewport()->width()/2,GetRenderContext()->GetView()->getCamera()->getViewport()->height()/2);
		osg::Vec3d vecCenterLLH;
		FeUtil::ScreenXY2DegreeLLH(GetRenderContext(),vecCenter.x(),vecCenter.y(),vecCenterLLH);
		auto pTNode = osgEarth::findRelativeNodeOfType<osgEarth::Drivers::MPTerrainEngine::TerrainNode>(GetRenderContext()->GetView()->getCamera() );
		if(pTNode)
		{
			if (pTNode->getNumChildren() > 0)
			{
				if ( pTNode->getChild(0))
				{
					ContourTileNodeVisitor nv;
					auto pCTileNode = nv.GetTileNode(osg::Vec2d(vecCenterLLH.x(), vecCenterLLH.y()), pTNode->getChild(0));
					if(pCTileNode)
					{
						nTileLod = pCTileNode->getKey().getLOD();
					}
				}
			}		
		}
		double n = ::powf(2.0, nTileLod);
		double dDeltaX = 180.0f / n;
		double dDeltaY = 180.0f / n;
		int nLDTileX = (m_vecLD.x() + 180.0) / dDeltaX;
		int nLDTileY = (90.0 - m_vecLD.y()) / dDeltaY;
		int nLUTileX = (m_vecLU.x() + 180.0) / dDeltaX;
		int nLUTileY = (90.0 - m_vecLU.y()) / dDeltaY;
		int nRUTileX = (m_vecRU.x() + 180.0) / dDeltaX;
		int nRUTileY = (90.0 - m_vecRU.y()) / dDeltaY;
		int nRDTileX = (m_vecRD.x() + 180.0) / dDeltaX;
		int nRDTileY = (90.0 - m_vecRD.y()) / dDeltaY;
		int nTileMinX = Min(Min(Min(nLDTileX,nLUTileX),nRUTileX),nRDTileX);
		int nTileMaxX = Max(Max(Max(nLDTileX,nLUTileX),nRUTileX),nRDTileX);
		int nTileMinY = Min(Min(Min(nLDTileY,nLUTileY),nRUTileY),nRDTileY);
		int nTileMaxY = Max(Max(Max(nLDTileY,nLUTileY),nRUTileY),nRDTileY);
		for (auto nTileX=nTileMinX;nTileX<=nTileMaxX;++nTileX)
		{
			for (auto nTileY=nTileMinY;nTileY<=nTileMaxY;++nTileY)
			{
				if (m_bRunning)
				{
					osg::ref_ptr<osg::HeightField> pHf = new osg::HeightField;//获取当前TileKey的高程
					pHf->allocate(17, 17); //16*16的高程数据？
					osgEarth::TileKey tileKey(nTileLod,nTileX,nTileY, m_pMap->getProfile());
					m_Elv.populateHeightField(pHf, tileKey, m_pMap->getProfileNoVDatum(), m_pMap->getMapOptions().elevationInterpolation().get(), 0L);
					if (pHf.valid()&&tileKey.valid())
					{
						osgEarth::GeoHeightField ghf(pHf, tileKey.getExtent());
						m_vecTileNode.push_back(ghf);
					}	
				}
				else
					return false;
			}
		}
		return true;
	}

	bool CContourResult::ComputePointDataLevel()
	{
		for (auto i=0;i<m_vecPointData.size();++i)
		{
			auto pData = m_vecPointData.at(i);
			for (auto j=0;j<m_vecTileNode.size();++j)
			{
				if (m_bRunning)
				{
					if (m_vecTileNode.at(j).getExtent().contains(pData->vecLLH))
					{
						auto ghf = m_vecTileNode.at(j);
						float fHei = 0.0;
						ghf.getElevation(m_pSrs, pData->vecLLH.x(), pData->vecLLH.y(), m_pMap->getMapOptions().elevationInterpolation().get(), m_pSrs, fHei);
						pData->vecLLH.z() = fHei;
						if (fHei>0)
						{
							pData->nLevel = int(fHei/m_nInterval);
						}
						else
						{
							pData->nLevel = int(fHei/m_nInterval)-1;
						}
						break;				
					}
				}
				else
					return false;
			}
		}
		return true;
	}

	bool CContourResult::CreatContourNode()
	{
		if (m_rpContourNode.valid())
		{
			removeChild(m_rpContourNode.get());
			m_rpContourNode.release();
		}
		if (m_rpTextGroup.valid())
		{
			if (m_rpTextGroup->getNumChildren()>0)
			{
				m_rpTextGroup->removeChildren(0,m_rpTextGroup->getNumChildren());
			}
		}
		else
		{
			m_rpTextGroup = new osg::Group;
		}
		auto mapLineData = GetLineDataByPointData(m_vecPointData );
		if (mapLineData.size()>0)
		{
			osgEarth::Features::FeatureList m_featureList;
			for (auto itrLine = mapLineData.begin();itrLine!=mapLineData.end();itrLine++)
			{
				auto listLineData = itrLine->second;
				while(listLineData.size()>0)
				{
					if (m_bRunning)
					{
						int nCount = 0;
						auto pLineData = *listLineData.begin();
						listLineData.erase(listLineData.begin());
						osg::ref_ptr<osgEarth::Symbology::LineString> rpLine = new osgEarth::Symbology::LineString;
						Check(pLineData,listLineData,rpLine,0,nCount);
						osg::ref_ptr<osgEarth::Features::Feature> rpLineFeature = new osgEarth::Features::Feature(rpLine, m_pSrs, osgEarth::Symbology::Style());
						rpLineFeature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;
						m_featureList.push_back(rpLineFeature);

						rpLine = new osgEarth::Symbology::LineString;
						Check(pLineData,listLineData,rpLine,0,nCount);
						rpLineFeature = new osgEarth::Features::Feature(rpLine, m_pSrs, osgEarth::Symbology::Style());
						rpLineFeature->geoInterp() = osgEarth::GEOINTERP_RHUMB_LINE;
						m_featureList.push_back(rpLineFeature);
						delete pLineData;
					}
					else
						return false;
				}		
			}
			m_rpContourNode = new osgEarth::Annotation::FeatureNode(m_pMapNode, m_featureList, m_Linestyle);
			m_rpContourNode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			m_rpContourNode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
			m_rpContourNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
			addChild(m_rpContourNode.get());
			return true;
		}
		else
			return false;
	}

	std::map<int,std::list<SLineData*>> CContourResult::GetLineDataByPointData(std::vector<SPointData*> vecPointData )
	{
		osg::Vec3d vecALLH(0.0,0.0,0.0),vecBLLH(0.0,0.0,0.0);
		std::map<int,std::list<SLineData*>> mapLineData;
		SPointData* pData1 = nullptr;
		SPointData* pData2 = nullptr;
		SPointData* pData3 = nullptr;
		SPointData* pData4 = nullptr;
		SPointData* pA1Data = nullptr;
		SPointData* pA2Data = nullptr;
		SPointData* pB1Data = nullptr;
		SPointData* pB2Data = nullptr;
		SPointData* pC1Data = nullptr;
		SPointData* pD1Data = nullptr;
		for (int nRow=0;nRow<(m_nRows-1);++nRow)
		{
			for (int nColumn=0;nColumn<(m_nColumns-1);++nColumn)
			{
				if (m_bRunning)
				{
					pData1 = vecPointData.at(nRow*(m_nColumns)+nColumn);
					pData2 = vecPointData.at(nRow*(m_nColumns)+nColumn+1);
					pData3 = vecPointData.at((nRow+1)*(m_nColumns)+nColumn);
					pData4 = vecPointData.at((nRow+1)*(m_nColumns)+nColumn+1);
					bool bStateABAA = pData1->nLevel==pData3->nLevel&&pData1->nLevel==pData4->nLevel&&pData1->nLevel!=pData2->nLevel;
					bool bStateAABA = pData1->nLevel==pData2->nLevel&&pData1->nLevel==pData4->nLevel&&pData1->nLevel!=pData3->nLevel;
					bool bStateAAAB = pData1->nLevel==pData2->nLevel&&pData1->nLevel==pData3->nLevel&&pData1->nLevel!=pData4->nLevel;
					bool bStateBAAA = pData2->nLevel==pData3->nLevel&&pData2->nLevel==pData4->nLevel&&pData1->nLevel!=pData2->nLevel;
					bool bStateAABB = pData1->nLevel==pData2->nLevel&&pData3->nLevel==pData4->nLevel&&pData1->nLevel!=pData4->nLevel;
					bool bStateABAB = pData1->nLevel==pData3->nLevel&&pData2->nLevel==pData4->nLevel&&pData1->nLevel!=pData4->nLevel;
					bool bStateABBA = pData1->nLevel==pData4->nLevel&&pData2->nLevel==pData3->nLevel&&pData1->nLevel!=pData2->nLevel;
					bool bStateABCA = pData1->nLevel==pData4->nLevel&&pData1->nLevel!=pData2->nLevel&&pData1->nLevel!=pData3->nLevel&&pData2->nLevel!=pData3->nLevel;
					bool bStateBAAC = pData2->nLevel==pData3->nLevel&&pData1->nLevel!=pData2->nLevel&&pData2->nLevel!=pData4->nLevel&&pData1->nLevel!=pData4->nLevel;
					bool bStateAABC = pData1->nLevel==pData2->nLevel&&pData1->nLevel!=pData3->nLevel&&pData1->nLevel!=pData4->nLevel&&pData3->nLevel!=pData4->nLevel;
					bool bStateBCAA = pData3->nLevel==pData4->nLevel&&pData1->nLevel!=pData3->nLevel&&pData2->nLevel!=pData3->nLevel&&pData1->nLevel!=pData2->nLevel;
					bool bStateABAC = pData1->nLevel==pData3->nLevel&&pData1->nLevel!=pData2->nLevel&&pData1->nLevel!=pData4->nLevel&&pData2->nLevel!=pData4->nLevel;
					bool bStateBACA = pData2->nLevel==pData4->nLevel&&pData1->nLevel!=pData2->nLevel&&pData2->nLevel!=pData3->nLevel&&pData1->nLevel!=pData3->nLevel;
					if (bStateABAA||bStateAABA||bStateAAAB||bStateBAAA)
					{
						if (bStateABAA)
						{
							pB1Data = pData2;
							pA1Data = pData1;
							pA2Data = pData4;
						}
						else if (bStateAABA)
						{
							pB1Data = pData3;
							pA1Data = pData4;
							pA2Data = pData1;
						}
						else if (bStateAAAB)
						{
							pB1Data = pData4;
							pA1Data = pData3;
							pA2Data = pData2;
						}
						else if (bStateBAAA)
						{
							pB1Data = pData1;
							pA1Data = pData2;
							pA2Data = pData3;
						}
						if (pB1Data->nLevel>pA1Data->nLevel)
						{
							for (int i=1;i<=(pB1Data->nLevel-pA1Data->nLevel);++i)
							{
								vecALLH = pA1Data->vecLLH+(pB1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA1Data->vecLLH.z());
								vecBLLH = pA2Data->vecLLH+(pB1Data->vecLLH-pA2Data->vecLLH)*((pA2Data->nLevel+i)*m_nInterval-pA2Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA2Data->vecLLH.z());
								PushLineData(i,pA1Data,pB1Data,pA2Data,pB1Data,vecALLH,vecBLLH,mapLineData);
							}	
						}
						else
						{
							for (int i=1;i<=(pA1Data->nLevel-pB1Data->nLevel);++i)
							{
								vecALLH = pB1Data->vecLLH+(pA1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pA1Data->vecLLH.z()-pB1Data->vecLLH.z());
								vecBLLH = pB1Data->vecLLH+(pA2Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pA2Data->vecLLH.z()-pB1Data->vecLLH.z());
								PushLineData(i,pB1Data,pA1Data,pB1Data,pA2Data,vecALLH,vecBLLH,mapLineData);
							}
						}
					}			
					if (bStateAABB||bStateABAB)
					{
						if (bStateAABB)
						{
							pA1Data = pData1;
							pA2Data = pData2;
							pB1Data = pData3;
							pB2Data = pData4;				
						}
						else if (bStateABAB)
						{
							pA1Data = pData1;
							pA2Data = pData3;
							pB1Data = pData2;
							pB2Data = pData4;
						}
						if ((pB1Data->nLevel-pA1Data->nLevel)>=1)
						{
							for (int i=1;i<=(pB1Data->nLevel-pA1Data->nLevel);++i)
							{
								vecALLH = pA1Data->vecLLH+(pB1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA1Data->vecLLH.z());
								vecBLLH = pA2Data->vecLLH+(pB2Data->vecLLH-pA2Data->vecLLH)*((pA2Data->nLevel+i)*m_nInterval-pA2Data->vecLLH.z())/(pB2Data->vecLLH.z()-pA2Data->vecLLH.z());
								PushLineData(i,pA1Data,pB1Data,pA2Data,pB2Data,vecALLH,vecBLLH,mapLineData);
							}
						}
						else if ((pA1Data->nLevel-pB1Data->nLevel)>=1)
						{
							for (int i=1;i<=(pA1Data->nLevel-pB1Data->nLevel);++i)
							{
								vecALLH = pB1Data->vecLLH+(pA1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pA1Data->vecLLH.z()-pB1Data->vecLLH.z());
								vecBLLH = pB2Data->vecLLH+(pA2Data->vecLLH-pB2Data->vecLLH)*((pB2Data->nLevel+i)*m_nInterval-pB2Data->vecLLH.z())/(pA2Data->vecLLH.z()-pB2Data->vecLLH.z());
								PushLineData(i,pB1Data,pA1Data,pB2Data,pA2Data,vecALLH,vecBLLH,mapLineData);
							}
						}		
					}
					if (bStateABBA)
					{
						pA1Data = pData1;
						pA2Data = pData4;
						pB1Data = pData2;
						pB2Data = pData3;
						if ((pB1Data->nLevel-pA1Data->nLevel)>=1)
						{
							for (int i=1;i<=(pB1Data->nLevel-pA1Data->nLevel);++i)
							{
								vecALLH = pA1Data->vecLLH+(pB1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA1Data->vecLLH.z());
								vecBLLH = pA1Data->vecLLH+(pB2Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pB2Data->vecLLH.z()-pA1Data->vecLLH.z());
								PushLineData(i,pA1Data,pB1Data,pA1Data,pA1Data,vecALLH,vecBLLH,mapLineData);
								vecALLH = pA2Data->vecLLH+(pB1Data->vecLLH-pA2Data->vecLLH)*((pA2Data->nLevel+i)*m_nInterval-pA2Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA2Data->vecLLH.z());
								vecBLLH = pA2Data->vecLLH+(pB2Data->vecLLH-pA2Data->vecLLH)*((pA2Data->nLevel+i)*m_nInterval-pA2Data->vecLLH.z())/(pB2Data->vecLLH.z()-pA2Data->vecLLH.z());
								PushLineData(i,pA2Data,pB1Data,pA2Data,pB2Data,vecALLH,vecBLLH,mapLineData);
							}
						}
						else if ((pA1Data->nLevel-pB1Data->nLevel)>=1)
						{
							for (int i=1;i<=(pA1Data->nLevel-pB1Data->nLevel);++i)
							{
								vecALLH = pB1Data->vecLLH+(pA1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pA1Data->vecLLH.z()-pB1Data->vecLLH.z());
								vecBLLH = pB1Data->vecLLH+(pA2Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pA2Data->vecLLH.z()-pB1Data->vecLLH.z());
								PushLineData(i,pB1Data,pA1Data,pB1Data,pA2Data,vecALLH,vecBLLH,mapLineData);
								vecALLH = pB2Data->vecLLH+(pA1Data->vecLLH-pB2Data->vecLLH)*((pB2Data->nLevel+i)*m_nInterval-pB2Data->vecLLH.z())/(pA1Data->vecLLH.z()-pB2Data->vecLLH.z());
								vecBLLH = pB2Data->vecLLH+(pA2Data->vecLLH-pB2Data->vecLLH)*((pB2Data->nLevel+i)*m_nInterval-pB2Data->vecLLH.z())/(pA2Data->vecLLH.z()-pB2Data->vecLLH.z());
								PushLineData(i,pB2Data,pA1Data,pB2Data,pA2Data,vecALLH,vecBLLH,mapLineData);
							}
						}
					}
					if (bStateABCA||bStateBAAC)
					{
						if (bStateABCA)
						{
							pA1Data = pData1;
							pA2Data = pData4;
							pB1Data = pData2;
							pC1Data = pData3;
						}
						else if (bStateBAAC)
						{
							pA1Data = pData2;
							pA2Data = pData3;
							pB1Data = pData1;
							pC1Data = pData4;
						}
						if ((pA1Data->nLevel-pB1Data->nLevel)>=1)
						{
							for (int i=1;i<=(pA1Data->nLevel-pB1Data->nLevel);++i)
							{
								vecALLH = pB1Data->vecLLH+(pA1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pA1Data->vecLLH.z()-pB1Data->vecLLH.z());
								vecBLLH = pB1Data->vecLLH+(pA2Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pA2Data->vecLLH.z()-pB1Data->vecLLH.z());
								PushLineData(i,pB1Data,pA1Data,pB1Data,pA2Data,vecALLH,vecBLLH,mapLineData);
							}
						}
						else if ((pB1Data->nLevel-pA1Data->nLevel)>=1)
						{
							for (int i=1;i<=(pB1Data->nLevel-pA1Data->nLevel);++i)
							{
								vecALLH = pA1Data->vecLLH+(pB1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA1Data->vecLLH.z());
								vecBLLH = pA2Data->vecLLH+(pB1Data->vecLLH-pA2Data->vecLLH)*((pA2Data->nLevel+i)*m_nInterval-pA2Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA2Data->vecLLH.z());
								PushLineData(i,pA1Data,pB1Data,pA2Data,pB1Data,vecALLH,vecBLLH,mapLineData);
							}
						}
						if ((pA1Data->nLevel-pC1Data->nLevel)>=1)
						{
							for (int i=1;i<=(pA1Data->nLevel-pC1Data->nLevel);++i)
							{
								vecALLH = pC1Data->vecLLH+(pA1Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+i)*m_nInterval-pC1Data->vecLLH.z())/(pA1Data->vecLLH.z()-pC1Data->vecLLH.z());
								vecBLLH = pC1Data->vecLLH+(pA2Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+i)*m_nInterval-pC1Data->vecLLH.z())/(pA2Data->vecLLH.z()-pC1Data->vecLLH.z());
								PushLineData(i,pC1Data,pA1Data,pC1Data,pA2Data,vecALLH,vecBLLH,mapLineData);
							}
						}
						else if ((pC1Data->nLevel-pA1Data->nLevel)>=1)
						{
							for (int i=1;i<=(pC1Data->nLevel-pA1Data->nLevel);++i)
							{
								vecALLH = pA1Data->vecLLH+(pC1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pC1Data->vecLLH.z()-pA1Data->vecLLH.z());
								vecBLLH = pA2Data->vecLLH+(pC1Data->vecLLH-pA2Data->vecLLH)*((pA2Data->nLevel+i)*m_nInterval-pA2Data->vecLLH.z())/(pC1Data->vecLLH.z()-pA2Data->vecLLH.z());
								PushLineData(i,pA1Data,pC1Data,pA2Data,pC1Data,vecALLH,vecBLLH,mapLineData);
							}
						}
					}		
					if (bStateAABC||bStateBCAA||bStateABAC||bStateBACA)
					{
						if(bStateAABC)
						{
							pA1Data = pData1;
							pA2Data = pData2;
							pB1Data = pData3;
							pC1Data = pData4;
						}
						else if (bStateBCAA)
						{
							pA1Data = pData3;
							pA2Data = pData4;
							pB1Data = pData1;
							pC1Data = pData2;
						}
						else if (bStateABAC)
						{
							pA1Data = pData1;
							pA2Data = pData3;
							pB1Data = pData2;
							pC1Data = pData4;
						}
						else if (bStateBACA)
						{
							pA1Data = pData2;
							pA2Data = pData4;
							pB1Data = pData1;
							pC1Data = pData3;
						}
						if ((pB1Data->nLevel-pA1Data->nLevel)>=1)
						{
							if ((pC1Data->nLevel-pA2Data->nLevel)>=1)
							{
								if ((pB1Data->nLevel-pC1Data->nLevel)>=1)
								{
									for (int i=1,j=1;i<=(pB1Data->nLevel-pA1Data->nLevel);++i)
									{
										if (i<=(pC1Data->nLevel-pA2Data->nLevel))
										{
											vecALLH = pA1Data->vecLLH+(pB1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA1Data->vecLLH.z());
											vecBLLH = pA2Data->vecLLH+(pC1Data->vecLLH-pA2Data->vecLLH)*((pA2Data->nLevel+i)*m_nInterval-pA2Data->vecLLH.z())/(pC1Data->vecLLH.z()-pA2Data->vecLLH.z());
											PushLineData(i,pA1Data,pB1Data,pA2Data,pC1Data,vecALLH,vecBLLH,mapLineData);
										}
										else
										{
											vecALLH = pA1Data->vecLLH+(pB1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA1Data->vecLLH.z());
											vecBLLH = pC1Data->vecLLH+(pB1Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+j)*m_nInterval-pC1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pC1Data->vecLLH.z());
											PushLineData(i,pA1Data,pB1Data,pC1Data,pB1Data,vecALLH,vecBLLH,mapLineData);
											++j;
										}
									}
								}
								else if ((pC1Data->nLevel-pB1Data->nLevel)>=1)
								{
									for (int i=1,j=1;i<=(pC1Data->nLevel-pA2Data->nLevel);++i)
									{
										if (i<=(pB1Data->nLevel-pA1Data->nLevel))
										{
											vecALLH = pA2Data->vecLLH+(pC1Data->vecLLH-pA2Data->vecLLH)*((pA2Data->nLevel+i)*m_nInterval-pA2Data->vecLLH.z())/(pC1Data->vecLLH.z()-pA2Data->vecLLH.z());
											vecBLLH = pA1Data->vecLLH+(pB1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA1Data->vecLLH.z());
											PushLineData(i,pA2Data,pC1Data,pA1Data,pB1Data,vecALLH,vecBLLH,mapLineData);
										}
										else
										{
											vecALLH = pA2Data->vecLLH+(pC1Data->vecLLH-pA2Data->vecLLH)*((pA2Data->nLevel+i)*m_nInterval-pA2Data->vecLLH.z())/(pC1Data->vecLLH.z()-pA2Data->vecLLH.z());
											vecBLLH = pB1Data->vecLLH+(pC1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+j)*m_nInterval-pB1Data->vecLLH.z())/(pC1Data->vecLLH.z()-pB1Data->vecLLH.z());
											PushLineData(i,pA2Data,pC1Data,pB1Data,pC1Data,vecALLH,vecBLLH,mapLineData);
											++j;
										}
									}
								}

							}
							else if ((pA2Data->nLevel-pC1Data->nLevel)>=1)
							{
								for (int i=1,j=1;i<=(pB1Data->nLevel-pC1Data->nLevel);++i)
								{
									if (i<=(pA2Data->nLevel-pC1Data->nLevel))
									{
										vecALLH = pC1Data->vecLLH+(pA2Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+i)*m_nInterval-pC1Data->vecLLH.z())/(pA2Data->vecLLH.z()-pC1Data->vecLLH.z());
										vecBLLH = pC1Data->vecLLH+(pB1Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+i)*m_nInterval-pC1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pC1Data->vecLLH.z());
										PushLineData(i,pC1Data,pA2Data,pC1Data,pB1Data,vecALLH,vecBLLH,mapLineData);
									}
									else
									{
										vecALLH = pC1Data->vecLLH+(pB1Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+i)*m_nInterval-pC1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pC1Data->vecLLH.z());
										vecBLLH = pA1Data->vecLLH+(pB1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+j)*m_nInterval-pA1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA1Data->vecLLH.z());
										PushLineData(i,pC1Data,pB1Data,pA1Data,pB1Data,vecALLH,vecBLLH,mapLineData);
										++j;
									}
								}
							}			
						}
						else if ((pA1Data->nLevel-pB1Data->nLevel)>=1)
						{
							if ((pA2Data->nLevel-pC1Data->nLevel)>=1)
							{
								if ((pB1Data->nLevel-pC1Data->nLevel)>=1)
								{
									for (int i=1,j=1;i<=(pA2Data->nLevel-pC1Data->nLevel);++i)
									{
										if (i<=(pB1Data->nLevel-pC1Data->nLevel))
										{
											vecALLH = pC1Data->vecLLH+(pA2Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+i)*m_nInterval-pC1Data->vecLLH.z())/(pA2Data->vecLLH.z()-pC1Data->vecLLH.z());
											vecBLLH = pC1Data->vecLLH+(pB1Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+i)*m_nInterval-pC1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pC1Data->vecLLH.z());
											PushLineData(i,pC1Data,pA2Data,pC1Data,pB1Data,vecALLH,vecBLLH,mapLineData);
										}
										else
										{
											vecALLH = pC1Data->vecLLH+(pA2Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+i)*m_nInterval-pC1Data->vecLLH.z())/(pA2Data->vecLLH.z()-pC1Data->vecLLH.z());
											vecBLLH = pB1Data->vecLLH+(pA1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+j)*m_nInterval-pB1Data->vecLLH.z())/(pA1Data->vecLLH.z()-pB1Data->vecLLH.z());
											PushLineData(i,pC1Data,pA2Data,pB1Data,pA1Data,vecALLH,vecBLLH,mapLineData);
											++j;
										}
									}
								}
								else if ((pC1Data->nLevel-pB1Data->nLevel)>=1)
								{
									for (int i=1,j=1;i<=(pA1Data->nLevel-pB1Data->nLevel);++i)
									{
										if (i<=(pC1Data->nLevel-pB1Data->nLevel))
										{
											vecALLH = pB1Data->vecLLH+(pA1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pA1Data->vecLLH.z()-pB1Data->vecLLH.z());
											vecBLLH = pB1Data->vecLLH+(pC1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pC1Data->vecLLH.z()-pB1Data->vecLLH.z());
											PushLineData(i,pB1Data,pA1Data,pB1Data,pC1Data,vecALLH,vecBLLH,mapLineData);
										}
										else
										{
											vecALLH = pB1Data->vecLLH+(pA1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pA1Data->vecLLH.z()-pB1Data->vecLLH.z());
											vecBLLH = pC1Data->vecLLH+(pA2Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+j)*m_nInterval-pC1Data->vecLLH.z())/(pA2Data->vecLLH.z()-pC1Data->vecLLH.z());
											PushLineData(i,pB1Data,pA1Data,pC1Data,pA2Data,vecALLH,vecBLLH,mapLineData);
											++j;
										}
									}
								}
							}
							else if ((pC1Data->nLevel-pA2Data->nLevel)>=1)
							{
								for (int i=1,j=1;i<=(pC1Data->nLevel-pB1Data->nLevel);++i)
								{
									if (i<=(pA1Data->nLevel-pB1Data->nLevel))
									{
										vecALLH = pB1Data->vecLLH+(pC1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pC1Data->vecLLH.z()-pB1Data->vecLLH.z());
										vecBLLH = pB1Data->vecLLH+(pA1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pA1Data->vecLLH.z()-pB1Data->vecLLH.z());
										PushLineData(i,pB1Data,pC1Data,pB1Data,pA1Data,vecALLH,vecBLLH,mapLineData);
									}
									else
									{
										vecALLH = pB1Data->vecLLH+(pC1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+i)*m_nInterval-pB1Data->vecLLH.z())/(pC1Data->vecLLH.z()-pB1Data->vecLLH.z());
										vecBLLH = pA2Data->vecLLH+(pC1Data->vecLLH-pA2Data->vecLLH)*((pA2Data->nLevel+j)*m_nInterval-pA2Data->vecLLH.z())/(pC1Data->vecLLH.z()-pA2Data->vecLLH.z());
										PushLineData(i,pB1Data,pC1Data,pA2Data,pC1Data,vecALLH,vecBLLH,mapLineData);
										++j;
									}
								}
							}
						}
					}
					if(pData1->nLevel!=pData2->nLevel&&pData1->nLevel!=pData3->nLevel&&pData1->nLevel!=pData4->nLevel&&pData2->nLevel!=pData3->nLevel&&pData2->nLevel!=pData4->nLevel&&pData3->nLevel!=pData4->nLevel)
					{
						std::map<int,SPointData*> mapData;
						mapData[pData1->nLevel] = pData1;
						mapData[pData2->nLevel] = pData2;
						mapData[pData3->nLevel] = pData3;
						mapData[pData4->nLevel] = pData4;
						auto itr = mapData.begin();
						pA1Data = itr++->second;
						pB1Data = itr++->second;
						pC1Data = itr++->second;
						pD1Data = itr->second;
						if ((pA1Data==pData1&&pD1Data==pData4)||(pA1Data==pData2&&pD1Data==pData3)||(pA1Data==pData3&&pD1Data==pData2)||(pA1Data==pData4&&pD1Data==pData1))
						{
							for (int i=1,j=1,k=1;i<=(pD1Data->nLevel-pA1Data->nLevel);i++)
							{
								if (i<=(pC1Data->nLevel-pA1Data->nLevel))
								{
									if (i<=(pB1Data->nLevel-pA1Data->nLevel))
									{
										vecALLH = pA1Data->vecLLH+(pC1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pC1Data->vecLLH.z()-pA1Data->vecLLH.z());
										vecBLLH = pA1Data->vecLLH+(pB1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA1Data->vecLLH.z());
										PushLineData(i,pA1Data,pC1Data,pA1Data,pB1Data,vecALLH,vecBLLH,mapLineData);
									}
									else
									{
										vecALLH = pA1Data->vecLLH+(pC1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pC1Data->vecLLH.z()-pA1Data->vecLLH.z());
										vecBLLH = pB1Data->vecLLH+(pD1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+j)*m_nInterval-pB1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pB1Data->vecLLH.z());
										PushLineData(i,pA1Data,pC1Data,pB1Data,pD1Data,vecALLH,vecBLLH,mapLineData);
										++j;
									}
								}
								else
								{
									vecALLH = pC1Data->vecLLH+(pD1Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+k)*m_nInterval-pC1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pC1Data->vecLLH.z());
									vecBLLH = pB1Data->vecLLH+(pD1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+j)*m_nInterval-pB1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pB1Data->vecLLH.z());
									PushLineData(k,pC1Data,pD1Data,pB1Data,pD1Data,vecALLH,vecBLLH,mapLineData);
									++j;
									++k;
								}
							}
						}
						else if ((pA1Data==pData1&&pD1Data==pData2&&pB1Data==pData3)||(pA1Data==pData1&&pD1Data==pData3&&pB1Data==pData2)
							||(pA1Data==pData2&&pD1Data==pData1&&pB1Data==pData4)||(pA1Data==pData2&&pD1Data==pData4&&pB1Data==pData1)
							||(pA1Data==pData3&&pD1Data==pData1&&pB1Data==pData4)||(pA1Data==pData3&&pD1Data==pData4&&pB1Data==pData1)
							||(pA1Data==pData4&&pD1Data==pData2&&pB1Data==pData3)||(pA1Data==pData4&&pD1Data==pData3&&pB1Data==pData2))
						{
							for (int i=1,j=1,k=1;i<=(pD1Data->nLevel-pA1Data->nLevel);i++)
							{
								if (i<=(pC1Data->nLevel-pA1Data->nLevel))
								{
									if (i<=(pB1Data->nLevel-pA1Data->nLevel))
									{
										vecALLH = pA1Data->vecLLH+(pD1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pA1Data->vecLLH.z());
										vecBLLH = pA1Data->vecLLH+(pB1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pB1Data->vecLLH.z()-pA1Data->vecLLH.z());
										PushLineData(i,pA1Data,pD1Data,pA1Data,pB1Data,vecALLH,vecBLLH,mapLineData);
									}
									else
									{
										vecALLH = pA1Data->vecLLH+(pD1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pA1Data->vecLLH.z());
										vecBLLH = pB1Data->vecLLH+(pC1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+j)*m_nInterval-pB1Data->vecLLH.z())/(pC1Data->vecLLH.z()-pB1Data->vecLLH.z());
										PushLineData(i,pA1Data,pD1Data,pB1Data,pC1Data,vecALLH,vecBLLH,mapLineData);
										++j;
									}
								}
								else
								{
									vecALLH = pA1Data->vecLLH+(pD1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pA1Data->vecLLH.z());
									vecBLLH = pC1Data->vecLLH+(pD1Data->vecLLH-pC1Data->vecLLH)*((pC1Data->nLevel+k)*m_nInterval-pC1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pC1Data->vecLLH.z());
									PushLineData(i,pA1Data,pD1Data,pC1Data,pD1Data,vecALLH,vecBLLH,mapLineData);
									++k;
								}
							}
						}
						else if ((pA1Data==pData1&&pD1Data==pData2&&pC1Data==pData3)||(pA1Data==pData1&&pD1Data==pData3&&pC1Data==pData2)
							||(pA1Data==pData2&&pD1Data==pData1&&pC1Data==pData4)||(pA1Data==pData2&&pD1Data==pData4&&pC1Data==pData1)
							||(pA1Data==pData3&&pD1Data==pData1&&pC1Data==pData4)||(pA1Data==pData3&&pD1Data==pData4&&pC1Data==pData1)
							||(pA1Data==pData4&&pD1Data==pData2&&pC1Data==pData3)||(pA1Data==pData4&&pD1Data==pData3&&pC1Data==pData2))
						{
							for (int i=1,j=1;i<=(pD1Data->nLevel-pA1Data->nLevel);i++)
							{
								if (i<=(pC1Data->nLevel-pA1Data->nLevel))
								{
									if (i<=(pB1Data->nLevel-pA1Data->nLevel))
									{
										vecALLH = pA1Data->vecLLH+(pD1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pA1Data->vecLLH.z());
										vecBLLH = pA1Data->vecLLH+(pC1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pC1Data->vecLLH.z()-pA1Data->vecLLH.z());
										PushLineData(i,pA1Data,pD1Data,pA1Data,pC1Data,vecALLH,vecBLLH,mapLineData);
									}
									else
									{
										vecALLH = pA1Data->vecLLH+(pD1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pA1Data->vecLLH.z());
										vecBLLH = pA1Data->vecLLH+(pC1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pC1Data->vecLLH.z()-pA1Data->vecLLH.z());
										PushLineData(i,pA1Data,pD1Data,pA1Data,pC1Data,vecALLH,vecBLLH,mapLineData);
										vecALLH = pB1Data->vecLLH+(pD1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+j)*m_nInterval-pB1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pB1Data->vecLLH.z());
										vecBLLH = pB1Data->vecLLH+(pC1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+j)*m_nInterval-pB1Data->vecLLH.z())/(pC1Data->vecLLH.z()-pB1Data->vecLLH.z());
										PushLineData(j,pB1Data,pD1Data,pB1Data,pC1Data,vecALLH,vecBLLH,mapLineData);
										++j;
									}
								}
								else
								{
									vecALLH = pA1Data->vecLLH+(pD1Data->vecLLH-pA1Data->vecLLH)*((pA1Data->nLevel+i)*m_nInterval-pA1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pA1Data->vecLLH.z());
									vecBLLH = pB1Data->vecLLH+(pD1Data->vecLLH-pB1Data->vecLLH)*((pB1Data->nLevel+j)*m_nInterval-pB1Data->vecLLH.z())/(pD1Data->vecLLH.z()-pB1Data->vecLLH.z());
									PushLineData(i,pA1Data,pD1Data,pB1Data,pD1Data,vecALLH,vecBLLH,mapLineData);
									++j;
								}
							}
						}
					}
				}
				else
					return std::map<int,std::list<SLineData*>>();
			}
		}
		return mapLineData;
	}

	void CContourResult::PushLineData( int nCount,SPointData* pA1Data,SPointData* pA2Data,SPointData* pB1Data,SPointData* pB2Data,osg::Vec3d vecALLH,osg::Vec3d vecBLLH,std::map<int,std::list<SLineData*>>& mapLineData )
	{
		SLineData* pLineData = new SLineData;
		pLineData->nLevel = pA1Data->nLevel+nCount;
		pLineData->nAX1 = pA1Data->nColumn;
		pLineData->nAY1 = pA1Data->nRow;
		pLineData->nAX2 = pA2Data->nColumn;
		pLineData->nAY2 = pA2Data->nRow;
		pLineData->nBX1 = pB1Data->nColumn;
		pLineData->nBY1 = pB1Data->nRow;
		pLineData->nBX2 = pB2Data->nColumn;
		pLineData->nBY2 = pB2Data->nRow;
		pLineData->vecALLH = vecALLH;
		pLineData->vecBLLH = vecBLLH;
		if (mapLineData.count(pLineData->nLevel))
		{
			mapLineData[pLineData->nLevel].push_back(pLineData);
		}
		else
		{
			std::list<SLineData*> listLineData;;
			listLineData.push_back(pLineData);
			mapLineData[pLineData->nLevel] = listLineData;
		}
	}

	bool CContourResult::Check( SLineData* pLineData,std::list<SLineData*>& listLineData,osg::ref_ptr<osgEarth::Symbology::LineString> rpLine,int nPos,int &nCount )
	{
		auto itr = listLineData.begin();
		int nPos2 = 1;
		SLineData* pLineData2;
		for (;itr!=listLineData.end();itr++)
		{
			if (m_bRunning)
			{
				pLineData2 = *itr;
				if (nPos == 0 || nPos == 1)
				{
					if ((pLineData->nAX1==pLineData2->nAX1)&&(pLineData->nAY1==pLineData2->nAY1)&&
						(pLineData->nAX2==pLineData2->nAX2)&&(pLineData->nAY2==pLineData2->nAY2))
					{
						if (nPos == 0 && nCount%m_nTextInterval==0)
						{
							rpLine->push_back(pLineData->vecBLLH);
							CreatText(pLineData->vecBLLH,pLineData->nLevel);
						}
						rpLine->push_back(pLineData->vecALLH);
						nPos2 = 2;
						nCount++;
						if (nCount%m_nTextInterval==0)
						{
							CreatText(pLineData->vecALLH,pLineData->nLevel);
						}
						break;
					}
					else if ((pLineData->nAX1==pLineData2->nBX1)&&(pLineData->nAY1==pLineData2->nBY1)&&
						(pLineData->nAX2==pLineData2->nBX2)&&(pLineData->nAY2==pLineData2->nBY2))
					{
						if (nPos == 0 && nCount%m_nTextInterval==0)
						{
							rpLine->push_back(pLineData->vecBLLH);
							CreatText(pLineData->vecBLLH,pLineData->nLevel);
						}
						rpLine->push_back(pLineData->vecALLH);
						nPos2 = 1;
						nCount++;
						if (nCount%m_nTextInterval==0)
						{
							CreatText(pLineData->vecALLH,pLineData->nLevel);
						}
						break;
					}
				}
				if (nPos == 0 || nPos == 2)
				{
					if ((pLineData->nBX1==pLineData2->nAX1)&&(pLineData->nBY1==pLineData2->nAY1)&&
						(pLineData->nBX2==pLineData2->nAX2)&&(pLineData->nBY2==pLineData2->nAY2))
					{
						if (nPos == 0 && nCount%m_nTextInterval==0)
						{
							rpLine->push_back(pLineData->vecALLH);
							CreatText(pLineData->vecALLH,pLineData->nLevel);
						}
						rpLine->push_back(pLineData->vecBLLH);
						nPos2 = 2;
						nCount++;
						if (nCount%m_nTextInterval==0)
						{
							CreatText(pLineData->vecBLLH,pLineData->nLevel);
						}
						break;
					}
					else if ((pLineData->nBX1==pLineData2->nBX1)&&(pLineData->nBY1==pLineData2->nBY1)&&
						(pLineData->nBX2==pLineData2->nBX2)&&(pLineData->nBY2==pLineData2->nBY2))
					{
						if (nPos == 0 && nCount%m_nTextInterval==0)
						{
							rpLine->push_back(pLineData->vecALLH);
							CreatText(pLineData->vecALLH,pLineData->nLevel);
						}
						rpLine->push_back(pLineData->vecBLLH);
						nPos2 = 1;
						nCount++;
						if (nCount%m_nTextInterval==0)
						{
							CreatText(pLineData->vecBLLH,pLineData->nLevel);
						}
						break;
					}
				}
			}
			else
				return false;
		}
		if (itr!=listLineData.end())
		{
			listLineData.erase(itr);	
			if (!Check(pLineData2,listLineData,rpLine,nPos2,nCount))
				return false;
		}
		else
		{
			if (nPos == 0)
			{
				rpLine->push_back(pLineData->vecALLH);
				rpLine->push_back(pLineData->vecBLLH);
				CreatText(pLineData->vecALLH,pLineData->nLevel);
			}
			else if (nPos == 1)
			{
				rpLine->push_back(pLineData->vecALLH);
			}
			else if (nPos == 2)
			{
				rpLine->push_back(pLineData->vecBLLH);
			}		
		}
		return true;
	}

	void CContourResult::CreatText( osg::Vec3d vecLLH,int nLevel )
	{
		nMinContourValue = min(nLevel * m_nInterval, nMinContourValue);
		nMaxContourValue = max(nLevel * m_nInterval, nMaxContourValue);

		std::stringstream ss;
		ss<<nLevel*m_nInterval;
		// 		osgText::Text* m_text = new osgText::Text;
		// 		m_text->setFont("simhei.ttf");
		// 		m_text->setCharacterSize(20);
		// 		m_text->setText(ss.str()/*, osgText::String::ENCODING_UTF8*/);
		// 		m_text->setColor(osg::Vec4d(1.0,0.0,0.0,0.8));
		// 		m_text->setAlignment(osgText::TextBase::LEFT_CENTER);
		// 		m_text->setPosition(osg::Vec3d(0,0,0));
		// 		m_text->setAxisAlignment(osgText::TextBase::SCREEN);
		// 		m_text->setCharacterSizeMode(osgText::TextBase::SCREEN_COORDS);
		// 		m_text->setAutoRotateToScreen(true);
		// 		m_text->setFontResolution(64.0f,64.0f);
		// 		m_text->setDataVariance(osg::Object::DYNAMIC);
		// 		m_text->setBackdropType(osgText::Text::NONE);
		// 		m_text->setColorGradientMode(osgText::Text::SOLID);
		// 		osg::Geode* pGeode = new osg::Geode;
		// 		osgEarth::Decluttering::setEnabled( m_text->getOrCreateStateSet(), true );//设置规避效果
		// 		pGeode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		// 		pGeode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		// 		pGeode->getOrCreateStateSet()->setRenderingHint(20);
		// 		pGeode->getOrCreateStateSet()->setMode(GL_SMOOTH, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		// 		
		// 		//pGeode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		// 		osg::ref_ptr<osg::Depth> pDepth = new osg::Depth();
		// 		pDepth->setWriteMask(false);
		// 		pGeode->getOrCreateStateSet()->setAttribute(pDepth, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		// 		pGeode->addDrawable(m_text);
		// 		osg::MatrixTransform* pMT = new osg::MatrixTransform;
		// 		osg::Vec3d vecXYZ;
		// 		FeUtil::DegreeLLH2XYZ(GetRenderContext(),vecLLH,vecXYZ);
		// 		pMT->setMatrix(osg::Matrix::translate(vecXYZ+osg::Vec3d(0,0,1000)));
		// 		pMT->addChild(pGeode);	
		// 		m_rpTextGroup->addChild(pMT);
		osg::ref_ptr<osgEarth::Annotation::LabelNode> ln = new osgEarth::Annotation::LabelNode(m_pMapNode, osgEarth::GeoPoint(m_pSrs, vecLLH.x(),vecLLH.y(),vecLLH.z(), osgEarth::ALTMODE_ABSOLUTE), ss.str(), m_LabelStyle);
		m_rpTextGroup->addChild(ln.get());
	}

	int CContourResult::Min( int nA, int nB )
	{
		return nA<nB?nA:nB;
	}

	int CContourResult::Max( int nA, int nB )
	{
		return nA>nB?nA:nB;
	}

	void CContourResult::SetContourInterval( int nInterval )
	{
		m_nInterval = nInterval;
	}

	int CContourResult::GetContourInterval()
	{
		return m_nInterval;
	}

	void CContourResult::SetRow( int nRow )
	{
		m_nRows = nRow;
	}

	int CContourResult::GetRow()
	{
		return m_nRows;
	}

	void CContourResult::SetColumn( int nColumn )
	{
		m_nColumns = nColumn;
	}

	int CContourResult::GetColumn()
	{
		return m_nColumns;
	}

	void CContourResult::Stop()
	{
		m_bRunning = false;
	}

	int CContourResult::GetMaxContourValue()
	{
		return nMaxContourValue;
	}

	int CContourResult::GetMinContourValue()
	{
		//nMinContourValue初始化为100000
		if (nMinContourValue == 100000)
		{
			return 0;
		}
		return nMinContourValue;
	}

}

namespace FeMeasure
{
	CContourMeasure::CContourMeasure( FeUtil::CRenderContext* pContext ) : CMeasure(pContext)
	{
		SetKey("ContourMeasure");
		SetTitle("等高线");
		MeasureType() = EM_Contour;
		CContourResult* pContourResult = dynamic_cast<CContourResult*>(GetOrCreateCurrentResult());
	}


	CContourMeasure::~CContourMeasure()
	{

	}

	void CContourMeasure::AddCtrlPointImplement( const osg::Vec3d& vecPosLLH )
	{
		if(Vertexes().size() == 2) 
			Vertexes().back() = vecPosLLH;
		else
			Vertexes().push_back(vecPosLLH);
	}

	void CContourMeasure::SetCtrlPointsImplement( const osg::Vec3dArray& vecPointsLLH )
	{
		if(!vecPointsLLH.empty())
		{
			Vertexes().clear();
			Vertexes().push_back(vecPointsLLH.front());
			Vertexes().push_back(vecPointsLLH.back());
		}
	}

	void CContourMeasure::UpdateContour(int nInterval,int nRow,int nColumn,bool bUpdateTile)
	{
		CContourResult* pContourResult = dynamic_cast<CContourResult*>(GetOrCreateCurrentResult());
		if (pContourResult)
		{
			pContourResult->SetContourInterval(nInterval);
			pContourResult->SetRow(nRow);
			pContourResult->SetColumn(nColumn);
			pContourResult->UpdateContourNode(bUpdateTile);
		}	
	}

	int CContourMeasure::GetContourInterval()
	{
		CContourResult* pContourResult = dynamic_cast<CContourResult*>(GetOrCreateCurrentResult());
		if (pContourResult)
		{
			return pContourResult->GetContourInterval();
		}
		else
			return 0;
	}

	int CContourMeasure::GetContourRow()
	{
		CContourResult* pContourResult = dynamic_cast<CContourResult*>(GetOrCreateCurrentResult());
		if (pContourResult)
		{
			return pContourResult->GetRow();
		}
		else
			return 0;
	}

	int CContourMeasure::GetContourColumn()
	{
		CContourResult* pContourResult = dynamic_cast<CContourResult*>(GetOrCreateCurrentResult());
		if (pContourResult)
		{
			return pContourResult->GetColumn();
		}
		else
			return 0;
	}

	void CContourMeasure::CancelContour()
	{
		CContourResult* pContourResult = dynamic_cast<CContourResult*>(GetOrCreateCurrentResult());
		if (pContourResult)
		{
			pContourResult->Stop();
		}
	}
}
