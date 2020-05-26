/**************************************************************************************************
* @file DoveTailStraightArrow.h
* @note 燕尾直箭头(两点)图元基类
* @author L00091
* @data 2017-11-20
**************************************************************************************************/
#ifndef DOVETAIL_STRAIGHT_ARROW_H
#define DOVETAIL_STRAIGHT_ARROW_H

#include <FePlots/Export.h>
#include <FePlots/PolygonPlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CDovetailArrowOption
	  * @brief 燕尾直箭头图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CDoveTailStraightArrowOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDoveTailStraightArrowOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CDoveTailStraightArrowOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class CDoveTailStraightArrow
	  * @brief 燕尾直箭头图元基类
	  * @note 主要用于绘制燕尾直箭头
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CDoveTailStraightArrow : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDoveTailStraightArrow(FeUtil::CRenderContext *pRender,CDoveTailStraightArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CDoveTailStraightArrow(void);

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
		  * @brief 计算两个控制点时燕尾直箭头的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif