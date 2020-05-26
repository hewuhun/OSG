/**************************************************************************************************
* @file DovetailArrow.h
* @note 燕尾箭头图元基类
* @author w00040
* @data 2017-11-2
**************************************************************************************************/
#ifndef DOVETAIL_ARROW_H
#define DOVETAIL_ARROW_H

#include <FePlots/Export.h>
#include <FePlots/PolygonPlot.h>
#include <FeUtils/RenderContext.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CDovetailArrowOption
	  * @brief 燕尾箭头图元配置选项
	  * @author w00040
	*/
	class FEPLOTS_EXPORT CDovetailArrowOption : public CPolygonPlotOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDovetailArrowOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CDovetailArrowOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class CDovetailArrow
	  * @brief 燕尾箭头图元基类
	  * @note 主要用于绘制燕尾箭头
	  * @author w00040
	*/
	class FEPLOTS_EXPORT CDovetailArrow : public CPolygonPlot
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDovetailArrow(FeUtil::CRenderContext *pRender, CDovetailArrowOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CDovetailArrow(void);

		/**  
		  * @note 接受节点遍历器
		*/
		void Accept(FeExtNode::CExternNodeVisitor& nv);

		/**  
		  * @note 更新编辑器
		*/
		void UpdateEditor();

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

		/**  
		* @note计算三个或三个以上的控制点时的所有绘制点
		* 由于中间的控制点之间会进行差值，产生曲线效果，所以所需绘制点会很多
		* 这里使用的思想是将所有用户控制点连接起来形成一条折线段，
		* 然后在拐角进行曲线化处理（二次贝塞尔曲线差值），就形成了效果比较好的箭头 
		  * @param 参数 [in/out/inout]
		  * @param 参数 [in/out/inout]
		  * @return 返回值
		*/
		osg::Vec3dArray* CalculateMorePoints(osg::Vec3dArray* controlPois);
		/**  
		  * @brief 绘制燕尾箭头
		*/
		void DrawDovetailArrow();
	};
}

#endif