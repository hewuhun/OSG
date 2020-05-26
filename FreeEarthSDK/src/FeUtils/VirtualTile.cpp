#include <FeUtils/VirtualTile.h>

#include <osg/MatrixTransform>
#include <osgEarth/Locators>
#include <osgEarth/HeightFieldUtils>

namespace FeUtil
{
	CVirtualTile::CVirtualTile(MapNode* pMapNode)
		:m_opMapNode(pMapNode)
	{

	}

	CVirtualTile::~CVirtualTile()
	{

	}

	osg::Node* CVirtualTile::CreateTile( double west, double south, double east, double north, osg::Vec3d cameraPos )
	{
		//判断吗MapNode是否可用
		if (false ==m_opMapNode.valid())
		{
			return NULL;
		}

		//根据范围计算出所在的瓦片TileKey
		osgEarth::GeoExtent geoExtent(m_opMapNode->getMapSRS(), west, south, east, north);
		TileKeys tileKeys;
		unsigned int  a = 0;
		m_opMapNode->getMap()->getProfile()->getIntersectingTiles(geoExtent, a , tileKeys);
		if(tileKeys.size() == 0)
		{
			return NULL;
		}

		//根据相机的位置计算与瓦片的距离从而得出相机的高度
		GeoPoint lowerLeft(geoExtent.getSRS(), geoExtent.xMin(), geoExtent.yMin(), 0.0, ALTMODE_ABSOLUTE);
		GeoPoint upperRight(geoExtent.getSRS(), geoExtent.xMax(), geoExtent.yMax(), 0.0, ALTMODE_ABSOLUTE);
		osg::Vec3d ll, ur;
		lowerLeft.toWorld( ll );
		upperRight.toWorld( ur );
		osg::Vec3d toCenter = osg::Vec3d((ur.x()-ll.x())/2.0+ll.x(), (ur.y()-ll.y())/2.0+ll.y(), (ur.z()-ll.z())/2.0+ll.z());
		double dCameraDis = (cameraPos-toCenter).length();

		//通过相机的位置信息计算需要加载的瓦片的Level
		int nLevel = tileKeys.at(0).getLevelOfDetail();
		CalculateLevel(dCameraDis, nLevel);

		//通过加载的级别对加载的瓦片进行重新计算
		TileKeys resultTileKeys;
		for (int i = 0; i < tileKeys.size(); i++)
		{
			CreateAssignTileKey(tileKeys.at(i), nLevel, resultTileKeys);
		}

		//将最总结果中的Tilekey去重
		TileKeys lastResultTileKeys;
		for (int i = 0; i < resultTileKeys.size(); i++)
		{
			bool bHas = false;
			for (int j = 0; j < lastResultTileKeys.size(); j++)
			{
				if (lastResultTileKeys.at(j) == resultTileKeys.at(i))
				{
					bHas = true;
					break;
				}
			}
			if (false == bHas)
			{
				lastResultTileKeys.push_back(resultTileKeys.at(i));
			}
		}

		osg::ref_ptr<osg::Group> pGroup = new osg::Group;
		//绘制每一个瓦片的Geometry
		for (int i = 0; i < resultTileKeys.size(); i++)
		{
			TileKey tileKey = resultTileKeys.at(i);
			if (tileKey.valid())
			{
				pGroup->addChild(ConstructTileNode(tileKey));
			}
		}        

		return pGroup.release();
	}

	osg::MatrixTransform* CVirtualTile::ConstructTileNode( TileKey tileKey )
	{
		//计算TileNode的MT的平移矩阵
		osg::ref_ptr<osg::MatrixTransform> pTileNode = new osg::MatrixTransform; 
		osg::ref_ptr<osg::Geode> pSurfaceGeode = new osg::Geode(); 
		osg::ref_ptr<osg::Geometry> pSurface = new osg::Geometry();  
		osg::ref_ptr<osg::Vec3dArray> pCoord = new osg::Vec3dArray();
		
		pSurfaceGeode->addDrawable( pSurface );   
		pSurface->setVertexArray(pCoord);
		pTileNode->addChild( pSurfaceGeode );

		//计算瓦片的中心点的世界坐标
		osg::Vec3d centerModel;
		osgEarth::MapInfo pMapInfo(m_opMapNode->getMap()); 
		osg::ref_ptr<osgEarth::GeoLocator> pGeoLocator = osgEarth::GeoLocator::createForKey(tileKey, pMapInfo);
		pGeoLocator->unitToModel( osg::Vec3(0.5f, 0.5f, 0.0), centerModel );  
		pTileNode->setMatrix( osg::Matrix::translate(centerModel) );

		//获取当前的地球的夸张系数和偏移系数
		double dScale = m_opMapNode->getTerrainEngine()->getTerrainOptions().verticalScale().get();
		double dOffset = m_opMapNode->getTerrainEngine()->getTerrainOptions().verticalOffset().get();

		//获取当前TileKey的高程
		osg::ref_ptr<osg::HeightField> pHf = new osg::HeightField();
		m_opMapNode->getMap()->getHeightField(tileKey, true, pHf);


		//计算顶点
		int nRow = pHf->getNumRows();
		int nCol = pHf->getNumColumns();
		for(unsigned j=0; j < nRow; ++j)
		{
			for(unsigned i=0; i < nCol; ++i)
			{
				unsigned int iv = j*nCol + i;
				osg::Vec3d ndc( ((double)i)/(double)(nCol-1), ((double)j)/(double)(nRow-1), 0.0);

				float heightValue = 0.0f;
				heightValue = osgEarth::HeightFieldUtils::getHeightAtNormalizedLocation( pHf.get(), ndc.x(), ndc.y(), osgEarth::ElevationInterpolation::INTERP_TRIANGULATE );

				ndc.z() = heightValue * dScale + dOffset;

				osg::Vec3d model;
				pGeoLocator->unitToModel( ndc, model );
				pCoord->push_back(model - centerModel);
			}
		}

		//顶点连接方式
		osg::DrawElements* elements = new osg::DrawElementsUShort(GL_TRIANGLES);
		elements->reserveElements((nCol-1) * (nRow-1) * 6);
		
		for(unsigned j=0; j<nRow-1; ++j)
		{
			for(unsigned i=0; i<nCol-1; ++i)
			{
				int i00 = j*nCol + i;
				int i01 = i00+nCol;

				int i10 = i00+1;
				int i11 = i01+1;

				elements->addElement(i01);
				elements->addElement(i00);
				elements->addElement(i11);

				elements->addElement(i00);
				elements->addElement(i10);
				elements->addElement(i11);
			}
		}
		
		pSurface->addPrimitiveSet(elements);

		return pTileNode.release();
	}

	void CVirtualTile::CreateAssignTileKey( TileKey tileKey, int nLod, TileKeys& tileKeys )
	{
		if(false == tileKey.valid())
		{
			return;
		}

		TileKey tempTileKey = tileKey;
		if (tempTileKey.getLevelOfDetail() == nLod)
		{
			tileKeys.push_back(tempTileKey);
			return;
		}

		if (tempTileKey.getLevelOfDetail() > nLod)
		{
			tempTileKey = tempTileKey.createParentKey();
			while (tempTileKey.getLevelOfDetail() > nLod)
			{
				tempTileKey = tempTileKey.createParentKey();
			}

			tileKeys.push_back(tempTileKey);
		}
		else if(tempTileKey.getLevelOfDetail() < nLod)
		{
			for(int i = 0; i < 4; i++)
			{
				TileKey childTileKey = tempTileKey.createChildKey(i);
				if (childTileKey.getLevelOfDetail() != nLod)
				{
					CreateAssignTileKey(childTileKey, nLod, tileKeys);
				}
				else
				{
					tileKeys.push_back(childTileKey);
				}
			}
		}
	}

	bool CVirtualTile::CalculateLevel( double dCameraDis, int& nLevel )
	{
		if (false == m_opMapNode.valid())
		{
			return false;
		}

		int nMaxLod = 17;//m_opMapNode->getTerrainEngine()->getTerrainOptions().maxLOD().get();
		if (dCameraDis <= 0.0)
		{
			nLevel = nMaxLod;
			return true;
		}
		int nMinTileRangeFactor = m_opMapNode->getTerrainEngine()->getTerrainOptions().minTileRangeFactor().get();
		TileKeys tileKeys;
		m_opMapNode->getMap()->getProfile()->getAllKeysAtLOD(0, tileKeys);
		if (tileKeys.size() > 0)
		{
			double dRadius = tileKeys.at(0).getExtent().getBoundingGeoCircle().getRadius();
			if (dCameraDis > dRadius*nMinTileRangeFactor)
			{
				TileKey tempTileKey = tileKeys.at(0).createParentKey();
				if (tempTileKey.valid())
				{
					while(dCameraDis >= tempTileKey.getExtent().getBoundingGeoCircle().getRadius()*nMinTileRangeFactor)
					{
						nLevel = tempTileKey.getLevelOfDetail();
						tempTileKey = tempTileKey.createParentKey();
					}
				}
			}
			else
			{
				TileKey tempTileKey = tileKeys.at(0).createChildKey(0);
				if (tempTileKey.valid())
				{
					while(dCameraDis <= tempTileKey.getExtent().getBoundingGeoCircle().getRadius()*nMinTileRangeFactor)
					{
						nLevel = tempTileKey.getLevelOfDetail();
						tempTileKey = tempTileKey.createChildKey(0);
					}
				}
			}
		}

		if (nLevel > nMaxLod)
		{
			nLevel = nMaxLod;
		}

		return true;
	}


}
