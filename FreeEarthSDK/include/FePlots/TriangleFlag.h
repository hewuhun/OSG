/**************************************************************************************************
* @file TriangleFlag.h
* @note 三角旗标图元基类
* @author L00091
* @data 2017-11-22
**************************************************************************************************/
#ifndef TRIANGLE_FLAG_H
#define TRIANGLE_FLAG_H

#include <FePlots/Export.h>
#include <FePlots/PolygonPlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CTriangleFlagOption
	  * @brief 三角旗标图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CTriangleFlagOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CTriangleFlagOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CTriangleFlagOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class CTriangleFlag
	  * @brief 三角旗标图元基类
	  * @note 主要用于绘制三角旗标
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CTriangleFlag : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CTriangleFlag(FeUtil::CRenderContext *pRender, CTriangleFlagOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CTriangleFlag(void);

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
		  * @brief 计算两个控制点时三角旗标的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif