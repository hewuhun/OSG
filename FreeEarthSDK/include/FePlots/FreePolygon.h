/**************************************************************************************************
* @file FreePolygon.h
* @note 手绘面图元基类
* @author L00091
* @data 2017-11-23
**************************************************************************************************/
#ifndef FREE_POLYGON_H
#define FREE_POLYGON_H

#include <FePlots/PolygonPlot.h>
#include <FePlots/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CFreePolygonOption
	  * @brief 手绘面图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CFreePolygonOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CFreePolygonOption();

		/**  
		  * @brief 析构函数
		*/
		~CFreePolygonOption();
	};
}

namespace FePlots
{
	/**
	  * @class CFreePolygon
	  * @brief 手绘面图元基类
	  * @note 主要用于绘制手绘面
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CFreePolygon : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CFreePolygon(FeUtil::CRenderContext *pRender, CFreePolygonOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CFreePolygon();

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
		  * @brief 计算多个控制点时手绘面的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif