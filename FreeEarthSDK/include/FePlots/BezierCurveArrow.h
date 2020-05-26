/**************************************************************************************************
* @file BezierCurveArrow.h
* @note 贝塞尔曲线箭头图元基类
* @author L00091
* @data 2017-11-21
**************************************************************************************************/
#ifndef BEZIER_CURVE_ARROW_H
#define BEZIER_CURVE_ARROW_H

#include <FePlots/LinePlot.h>
#include <FePlots/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CBezierCurveArrowOption
	  * @brief 贝塞尔曲线箭头图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CBezierCurveArrowOption : public CLinePlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CBezierCurveArrowOption();

		/**  
		  * @brief 析构函数
		*/
		~CBezierCurveArrowOption();
	};
}

namespace FePlots
{
	/**
	  * @class CBezierCurveArrow
	  * @brief 贝塞尔曲线箭头图元基类
	  * @note 主要用于绘制贝塞尔曲线箭头
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CBezierCurveArrow : public CLinePlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CBezierCurveArrow(FeUtil::CRenderContext *pRender, CBezierCurveArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CBezierCurveArrow();

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
		  * @brief 计算多个控制点时贝塞尔曲线箭头的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif