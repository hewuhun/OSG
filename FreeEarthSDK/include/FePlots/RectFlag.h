/**************************************************************************************************
* @file CRectFlag.h
* @note 直角旗标图元基类
* @author L00091
* @data 2017-11-20
**************************************************************************************************/
#ifndef RECT_FLAG_H
#define RECT_FLAG_H

#include <FePlots/Export.h>
#include <FePlots/PolygonPlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CRectFlagOption
	  * @brief 直角旗标图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CRectFlagOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CRectFlagOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CRectFlagOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class CRectFlag
	  * @brief 直角旗标图元基类
	  * @note 主要用于绘制直角旗标
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CRectFlag : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CRectFlag(FeUtil::CRenderContext *pRender, CRectFlagOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CRectFlag(void);

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
		  * @brief 计算两个控制点时燕尾箭头的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif