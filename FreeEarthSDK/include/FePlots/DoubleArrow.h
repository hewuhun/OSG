/**************************************************************************************************
* @file DoubleArrow.h
* @note 双箭头图元基类
* @author L00091
* @data 2017-11-20
**************************************************************************************************/
#ifndef DOUBLE_ARROW_H
#define DOUBLE_ARROW_H

#include <FePlots/PolygonPlot.h>
#include <FePlots/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class DoubleArrowOption
	  * @brief 双箭头图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CDoubleArrowOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDoubleArrowOption();

		/**  
		  * @brief 析构函数
		*/
		~CDoubleArrowOption();
	};
}

namespace FePlots
{
	/**
	  * @class DoubleArrow
	  * @brief 双箭头图元基类
	  * @note 主要用于绘制双箭头
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CDoubleArrow : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDoubleArrow(FeUtil::CRenderContext *pRender, CDoubleArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CDoubleArrow();

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
		  * @brief 计算多个控制点时双箭头的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* vecControlPoints);
	};
}

#endif