/**************************************************************************************************
* @file DiagonalArrow.h
* @note 斜箭头图元基类
* @author L00091
* @data 2017-11-14
**************************************************************************************************/
#ifndef DIAGONAL_ARROW_H
#define DIAGONAL_ARROW_H

#include <FePlots/Export.h>
#include <FePlots/PolygonPlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CDiagonalArrowOption
	  * @brief 斜箭头图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CDiagonalArrowOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDiagonalArrowOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CDiagonalArrowOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class CDiagonalArrow
	  * @brief 斜箭头图元基类
	  * @note 主要用于绘制斜箭头
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CDiagonalArrow : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDiagonalArrow(FeUtil::CRenderContext *pRender, CDiagonalArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CDiagonalArrow(void);

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
		  * @brief 计算两个控制点时斜箭头的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);

	};
}

#endif