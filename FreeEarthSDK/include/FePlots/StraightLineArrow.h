/**************************************************************************************************
* @file StraightLineArrow.h
* @note 直线箭头图元基类
* @author L00091
* @data 2017-11-23
**************************************************************************************************/
#ifndef STRAIGHT_LINE_ARROW_H
#define STRAIGHT_LINE_ARROW_H

#include <FePlots/Export.h>
#include <FePlots/LinePlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CStraightLineArrowOption
	  * @brief 直线箭头图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CStraightLineArrowOption : public CLinePlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CStraightLineArrowOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CStraightLineArrowOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class CStraightLineArrow
	  * @brief 直线箭头图元基类
	  * @note 主要用于绘制直线箭头
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CStraightLineArrow : public CLinePlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CStraightLineArrow(FeUtil::CRenderContext *pRender, CStraightLineArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CStraightLineArrow(void);

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
		  * @brief 计算两个控制点时直线箭头的所有绘制点
		*/
		virtual osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif