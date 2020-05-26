/**************************************************************************************************
* @file DiagonalMoreArrow.h
* @note 多点斜箭头图元基类
* @author L00091
* @data 2017-11-20
**************************************************************************************************/
#ifndef DIAGONAL_MORE_ARROW_H
#define DIAGONAL_MORE_ARROW_H

#include <FePlots/PolygonPlot.h>
#include <FePlots/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CDiagonalMoreArrowOption
	  * @brief 多点斜箭头图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CDiagonalMoreArrowOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDiagonalMoreArrowOption();

		/**  
		  * @brief 析构函数
		*/
		~CDiagonalMoreArrowOption();
	};
}

namespace FePlots
{
	/**
	  * @class CDiagonalMoreArrow
	  * @brief 多点斜箭头图元基类
	  * @note 主要用于绘制多点斜箭头
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CDiagonalMoreArrow : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDiagonalMoreArrow(FeUtil::CRenderContext *pRender, CDiagonalMoreArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CDiagonalMoreArrow();

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
		  * @brief 计算多个控制点时斜箭头的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif