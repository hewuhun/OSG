/**************************************************************************************************
* @file DovetailDiagonalMoreArrow.h
* @note 折线箭头图元基类
* @author L00091
* @data 2017-11-22
**************************************************************************************************/
#ifndef POLYLINE_ARROW_H
#define POLYLINE_ARROW_H

#include <FePlots/LinePlot.h>
#include <FePlots/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CPolyLineArrowOption
	  * @brief 折线箭头图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CPolyLineArrowOption : public CLinePlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CPolyLineArrowOption();

		/**  
		  * @brief 析构函数
		*/
		~CPolyLineArrowOption();
	};
}

namespace FePlots
{
	/**
	  * @class CPolyLineArrow
	  * @brief 折线箭头图元基类
	  * @note 主要用于绘制折线箭头
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CPolyLineArrow : public CLinePlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CPolyLineArrow(FeUtil::CRenderContext *pRender, CPolyLineArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CPolyLineArrow();

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
		  * @brief 计算多个控制点时折线箭头的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif