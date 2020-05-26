/**************************************************************************************************
* @file FreeLine.h
* @note 自由线图元基类
* @author L00091
* @data 2017-11-22
**************************************************************************************************/
#ifndef FREE_LINE_H
#define FREE_LINE_H

#include <FePlots/LinePlot.h>
#include <FePlots/Export.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CFreeLineOption
	  * @brief 自由线图元配置选项
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CFreeLineOption : public CLinePlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CFreeLineOption();

		/**  
		  * @brief 析构函数
		*/
		~CFreeLineOption();
	};
}

namespace FePlots
{
	/**
	  * @class CFreeLine
	  * @brief 自由线图元基类
	  * @note 主要用于绘制自由线
	  * @author L00091
	*/
	class FEPLOTS_EXPORT CFreeLine : public CLinePlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CFreeLine(FeUtil::CRenderContext *pRender, CFreeLineOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CFreeLine();

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
		  * @brief 计算多个控制点时自由线的所有绘制点
		*/
		osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints);
	};
}

#endif