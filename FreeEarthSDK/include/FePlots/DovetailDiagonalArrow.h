/**************************************************************************************************
* @file DovetailDiagonalArrow.h
* @note 燕尾斜箭头（两点）图元基类
* @author w00040
* @data 2017-11-2
**************************************************************************************************/
#ifndef DOVETAIL_DIAGONAL_ARROW_H
#define DOVETAIL_DIAGONAL_ARROW_H

#include <FePlots/Export.h>
#include <FePlots/PolygonPlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CDovetailDiagonalArrowOption
	  * @brief 燕尾斜箭头图元配置选项
	  * @author w00040
	*/
	class FEPLOTS_EXPORT CDovetailDiagonalArrowOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDovetailDiagonalArrowOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CDovetailDiagonalArrowOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class CDovetailDiagonalArrow
	  * @brief 燕尾斜箭头图元基类
	  * @note 主要用于绘制燕尾斜箭头
	  * @author w00040
	*/
	class FEPLOTS_EXPORT CDovetailDiagonalArrow : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDovetailDiagonalArrow(FeUtil::CRenderContext *pRender, CDovetailDiagonalArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CDovetailDiagonalArrow(void);

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
		  * @brief 计算两个控制点时燕尾箭头的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif