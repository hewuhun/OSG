/**************************************************************************************************
* @file CardinalCurveArrow.h
* @note Cardinal曲线箭头图元基类
* @author L00091
* @data 2017-11-21
**************************************************************************************************/
#ifndef CARDINAL_CURVE_ARROW_H
#define CARDINAL_CURVE_ARROW_H

#include <FePlots/LinePlot.h>
#include <FePlots/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CCardinalCurveArrowOption
	  * @brief Cardinal曲线箭头图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CCardinalCurveArrowOption : public CLinePlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CCardinalCurveArrowOption();

		/**  
		  * @brief 析构函数
		*/
		~CCardinalCurveArrowOption();
	};
}

namespace FePlots
{
	/**
	  * @class CCardinalCurveArrow
	  * @brief Cardinal曲线箭头图元基类
	  * @note 主要用于绘制Cardinal曲线箭头
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CCardinalCurveArrow : public CLinePlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CCardinalCurveArrow(FeUtil::CRenderContext *pRender, CCardinalCurveArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CCardinalCurveArrow();

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
		  * @brief 计算多个控制点时Cardinal曲线箭头的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif