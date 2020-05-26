/**************************************************************************************************
* @file PolygonPlot.h
* @note 面类型军标基类
* @author w00040
* @data 2017-11-1
**************************************************************************************************/
#ifndef POLYGON_PLOT_H
#define POLYGON_PLOT_H

#include <FeExtNode/ExPolygonNode.h>
#include <FeUtils/RenderContext.h>
#include <FePlots/Export.h>
#include <FeExtNode/ExternNodeVisitor.h>

namespace FePlots
{
	/**
	  * @class CPolygonPlotOption
	  * @brief 面类型军标配置选项
	  * @author w00040
	*/
	class FEPLOTS_EXPORT CPolygonPlotOption : public FeExtNode::CExPolygonNodeOption
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CPolygonPlotOption(void);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CPolygonPlotOption(void);
	};
}

namespace FePlots
{
	/**
	  * @class CPolygonPlot
	  * @brief 面类型军标图元基类
	  * @note 主要用于绘制面类型军标
	  * @author w00040
	*/
	class FEPLOTS_EXPORT CPolygonPlot : public FeExtNode::CExPolygonNode
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CPolygonPlot(FeUtil::CRenderContext *pRender, CPolygonPlotOption *opt);

		/**  
		  * @brief 析构函数
		*/
		~CPolygonPlot(void);

		/**  
		  * @note 接受节点遍历器，子类实现
		*/
		virtual void Accept(FeExtNode::CExternNodeVisitor& nv);

		/**  
		  * @note 创建编辑点
		*/
		virtual osg::Group* CreateEditPoints();

		/**  
		  * @brief 获取编辑点位置
		*/
		virtual osgEarth::GeoPoint GetEditPointPosition();

		/**
		  * @note 拖拽编辑器矩阵变化量
		*/
		virtual void DraggerUpdateMatrix(const osg::Matrix& matrix);

		/**  
		  * @note 设置军标的控制点集合  
		*/
		virtual void SetVertex(osg::Vec3dArray* pCoord);

		/**  
		  * @note 获取军标的控制点
		*/
		virtual void GetVertex(osg::Vec3dArray* pCoord);

		/**  
		  * @brief 计算箭头的所有绘制点,子类需要实现
		*/
		virtual osg::Vec3dArray* CalculateArrow(osg::Vec3dArray* pPoints) = 0;

		/**  
		  * @brief 末尾压入顶点，子类实现
		*/
		virtual bool PushBackVertex( const osg::Vec3d& vecCoord ) = 0;

		/**  
		  * @brief 替换末尾顶点，子类实现
		*/
		virtual void ReplaceBackVertex(const osg::Vec3d& vecCoord) = 0;

		/**  
		  * @note 删除指定索引的顶点 
		*/
		virtual bool DeleteVertex(int nIndex);

		/**  
		  * @note 获取顶点的数量 
		*/
		virtual int GetNumVertex();

		/**  
		  * @brief 清空控制点数组
		*/
		void CleanCrtlArray();

	protected:
		// 存储控制点
		osg::ref_ptr<osg::Vec3dArray>				m_rpCtrlArray;

		// 鼠标移动标识
		bool										m_bMouseMove;
	};
}

#endif