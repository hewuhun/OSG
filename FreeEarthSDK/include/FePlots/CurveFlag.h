/**************************************************************************************************
* @file CurveFlag.h
* @note 曲线旗标图元基类
* @author L00091
* @data 2017-11-23
**************************************************************************************************/
#ifndef CURVE_FLAG_H
#define CURVE_FLAG_H

#include <FePlots/Export.h>
#include <FePlots/PolygonPlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CCurveFlagOption
	  * @brief 曲线旗标图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CCurveFlagOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CCurveFlagOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CCurveFlagOption(void);
	};
}
namespace FePlots
{
	/**
	  * @class CCurveFlag
	  * @brief 曲线旗标图元基类
	  * @note 主要用于绘制曲面旗标
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CCurveFlag : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CCurveFlag(FeUtil::CRenderContext *pRender, CCurveFlagOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CCurveFlag(void);

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
		void PushBackVertex( const osg::Vec3d& vecCoord );

		/**  
		  * @brief 替换末尾顶点
		*/
		void ReplaceBackVertex(const osg::Vec3d& vecCoord);

		/**  
		  * @brief 计算两个控制点时曲线标旗的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* vecControlPoints);

	};
}

#endif

