/**************************************************************************************************
* @file DovetailDiagonalMoreArrow.h
* @note 燕尾多点斜箭头图元基类
* @author w00040
* @data 2017-11-13
**************************************************************************************************/
#ifndef DOVETAIL_DIAGONAL_MORE_ARROW_H
#define DOVETAIL_DIAGONAL_MORE_ARROW_H

#include <FePlots/PolygonPlot.h>
#include <FePlots/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CDovetailDiagonalMoreArrowOption
	  * @brief 燕尾多点斜箭头图元配置选项
	  * @author w00040
	*/
	class FEPLOTS_EXPORT CDovetailDiagonalMoreArrowOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDovetailDiagonalMoreArrowOption();

		/**  
		  * @brief 析构函数
		*/
		~CDovetailDiagonalMoreArrowOption();
	};
}

namespace FePlots
{
	/**
	  * @class CDovetailDiagonalMoreArrow
	  * @brief 燕尾多点斜箭头图元基类
	  * @note 主要用于绘制燕尾多点斜箭头
	  * @author w00040
	*/
	class FEPLOTS_EXPORT CDovetailDiagonalMoreArrow : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDovetailDiagonalMoreArrow(FeUtil::CRenderContext *pRender, CDovetailDiagonalMoreArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CDovetailDiagonalMoreArrow();

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
		  * @brief 计算多个控制点时燕尾斜箭头的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif