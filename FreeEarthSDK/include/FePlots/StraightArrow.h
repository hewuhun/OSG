/**************************************************************************************************
* @file StraightArrow.h
* @note 直箭头图元基类
* @author L00091
* @data 2017-11-22
**************************************************************************************************/
#ifndef STRAIGHT_ARROW_H
#define STRAIGHT_ARROW_H

#include <FePlots/Export.h>
#include <FePlots/PolygonPlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CStraightArrowOption
	  * @brief 直箭头图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CStraightArrowOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CStraightArrowOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CStraightArrowOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class CStraightArrow
	  * @brief 直箭头图元基类
	  * @note 主要用于绘制直箭头
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CStraightArrow : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CStraightArrow(FeUtil::CRenderContext *pRender, CStraightArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CStraightArrow(void);

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
		  * @brief 计算两个控制点时直箭头的所有绘制点
		*/
		virtual osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif