/**************************************************************************************************
* @file CloseCurve.h
* @note 闭合曲线图元基类
* @author L00091
* @data 2017-11-21
**************************************************************************************************/
#ifndef CLOSE_CURVE_H
#define CLOSE_CURVE_H

#include <FePlots/PolygonPlot.h>
#include <FePlots/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CCloseCurveOption
	  * @brief 闭合曲线图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CCloseCurveOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CCloseCurveOption();

		/**  
		  * @brief 析构函数
		*/
		~CCloseCurveOption();
	};
}

namespace FePlots
{
	/**
	  * @class CCloseCurve
	  * @brief 闭合曲线图元基类
	  * @note 主要用于绘制闭合曲线
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CCloseCurve : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CCloseCurve(FeUtil::CRenderContext *pRender, CCloseCurveOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CCloseCurve();

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
		  * @brief 计算多个控制点时闭合曲线的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif