/**************************************************************************************************
* @file ParallelSearch.h
* @note 平行搜索区类
* @author L00091
* @data 2017-11-14
**************************************************************************************************/
#ifndef PARALLEL_SEARCH_H
#define PARALLEL_SEARCH_H

#include <FePlots/Export.h>
#include <FePlots/LinePlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CParallelSearchOption
	  * @brief 平行搜索区图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CParallelSearchOption : public CLinePlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CParallelSearchOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CParallelSearchOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class CParallelSearch
	  * @brief 平行搜索区图元基类
	  * @note 主要用于绘制平行搜索区
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CParallelSearch : public CLinePlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CParallelSearch(FeUtil::CRenderContext *pRender, CParallelSearchOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CParallelSearch();

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
		  * @brief 计算多个控制点时平行搜索区的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif