/**************************************************************************************************
* @file StraightMoreArrow.h
* @note 多点直箭头图元基类
* @author L00091
* @data 2017-11-22
**************************************************************************************************/
#ifndef STRAIGHT_MORE_ARROW_H
#define STRAIGHT_MORE_ARROW_H

#include <FePlots/PolygonPlot.h>
#include <FePlots/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CStraightMoreArrowOption
	  * @brief 多点直箭头图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CStraightMoreArrowOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CStraightMoreArrowOption();

		/**  
		  * @brief 析构函数
		*/
		~CStraightMoreArrowOption();
	};
}

namespace FePlots
{
	/**
	  * @class CStraightMoreArrow
	  * @brief 多点直箭头图元基类
	  * @note 主要用于绘制燕尾多点直箭头
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CStraightMoreArrow : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CStraightMoreArrow(FeUtil::CRenderContext *pRender, CStraightMoreArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CStraightMoreArrow();

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
		  * @brief 计算多个控制点时直箭头的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif