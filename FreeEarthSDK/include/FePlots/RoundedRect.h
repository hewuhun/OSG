/**************************************************************************************************
* @file RoundedRect.h
* @note 圆角矩形图元基类
* @author L00091
* @data 2017-11-22
**************************************************************************************************/
#ifndef ROUNDED_RECT_H
#define ROUNDED_RECT_H

#include <FePlots/Export.h>
#include <FePlots/PolygonPlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class RoundedRectOption
	  * @brief 圆角矩形图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CRoundedRectOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CRoundedRectOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CRoundedRectOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class RoundedRect
	  * @brief 圆角矩形图元基类
	  * @note 主要用于绘制圆角矩形
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CRoundedRect : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CRoundedRect(FeUtil::CRenderContext *pRender, CRoundedRectOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CRoundedRect(void);

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
		  * @brief 计算两个控制点时圆角矩形的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
		/**  
		  * @brief 两者之间的最大值
		*/
		double TheMax(double a ,double b);
		/**  
		  * @brief 两者之间的最小值
		*/
		double TheMin(double a ,double b);
	};
}

#endif