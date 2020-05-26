/**************************************************************************************************
* @file VirtualTile.h
* @note 虚拟瓦片的实现类，可以指定想要得到的瓦片范围，从而获取地球瓦片
* @author c00005
* @data 2015-7-8
**************************************************************************************************/
#ifndef UTIL_VIRTUAL_TILE_H
#define UTIL_VIRTUAL_TILE_H

#include <FeUtils/Export.h>

#include <osgEarth/MapNode>

namespace FeUtil
{
	using namespace osgEarth;
    /**
      * @class CVirtualTile
      * @note 虚拟瓦片的构造
      * @author c00005
    */
    class FEUTIL_EXPORT CVirtualTile
    {
	public:
		typedef std::vector<TileKey> TileKeys;

    public:
        CVirtualTile(MapNode* pMapNode);

        ~CVirtualTile();

    public:
		/**  
		  * @note 通过给定的范围：最小经度、最小维度、最大经度、最大纬度、相机的位置构造Tile  
		  * @param west [in] 最小经度
		  * @param south [in] 最小纬度
		  * @param east [in] 最大经度
		  * @param north [in] 最大纬度
		  * @param cameraPos [in] 相机的位置
		  * @return 返回值
		*/
		osg::Node* CreateTile( double west, double south, double east, double north, osg::Vec3d cameraPos );

	protected:
		osg::MatrixTransform* ConstructTileNode(TileKey tileKey);

		void CreateAssignTileKey(TileKey tileKey, int nLod, TileKeys& tileKeys);

		bool CalculateLevel( double dCameraDis, int& nLevel);

    protected:
		osg::observer_ptr<MapNode> m_opMapNode;
    };
}

#endif//UTIL_VIRTUAL_TILE_H
