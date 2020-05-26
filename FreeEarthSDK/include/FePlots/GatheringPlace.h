/**************************************************************************************************
* @file GatheringPlace.h
* @note 聚集区图元基类
* @author L00091
* @data 2017-11-20
**************************************************************************************************/
#ifndef GATHERING_PLACE_H
#define GATHERING_PLACE_H

#include <FePlots/Export.h>
#include <FePlots/PolygonPlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CGatheringPlaceOption
	  * @brief 聚集区图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CGatheringPlaceOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CGatheringPlaceOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CGatheringPlaceOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class CGatheringPlace
	  * @brief 聚集区图元基类
	  * @note 主要用于绘制聚集区
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CGatheringPlace : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CGatheringPlace(FeUtil::CRenderContext *pRender, CGatheringPlaceOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CGatheringPlace(void);

		/**  
		  * @note 接受节点遍历器
		*/
		void Accept(FeExtNode::CExternNodeVisitor& nv);

		/**
		  * @note 拖拽编辑器矩阵变化量
		*/
		virtual void DraggerUpdateMatrix(const osg::Matrix& matrix);

		/**  
		  * @brief 末尾压入顶点
		*/
		bool PushBackVertex( const osg::Vec3d& vecCoord );

		/**  
		  * @brief 替换末尾顶点
		*/
		void ReplaceBackVertex(const osg::Vec3d& vecCoord);

		/**  
		  * @brief 计算两个控制点时聚集区的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif