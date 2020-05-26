/**************************************************************************************************
* @file SectorSearch.h
* @note 扇形搜索区图元基类
* @author L00091
* @data 2017-11-23
**************************************************************************************************/
#ifndef SECTOR_SEARCH_H
#define SECTOR_SEARCH_H

#include <FePlots/LinePlot.h>
#include <FePlots/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CSectorSearchOption
	  * @brief 扇形搜索区图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CSectorSearchOption : public CLinePlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CSectorSearchOption();

		/**  
		  * @brief 析构函数
		*/
		~CSectorSearchOption();
	};
}

namespace FePlots
{
	/**
	  * @class CSectorSearch
	  * @brief 扇形搜索区图元基类
	  * @note 主要用于绘制扇形搜索区
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CSectorSearch : public CLinePlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CSectorSearch(FeUtil::CRenderContext *pRender, CSectorSearchOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CSectorSearch();

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
		  * @brief 计算两个控制点时扇形搜索区的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif