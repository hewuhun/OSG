/**************************************************************************************************
* @file ExGridNode.h
* @note 网格绘制类，用于绘制贴地网格图元
* @author g00034
* @data 2018-10-15
**************************************************************************************************/
#ifndef FE_EXTERN_GRID_NODE_H
#define FE_EXTERN_GRID_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExLodNode.h>

#include <osgEarthFeatures/Feature>
#include <osgEarthAnnotation/FeatureNode>

namespace FeUtil
{
	class CRenderContext;
}

namespace FeExtNode
{
	/**
	* @class CExGridNodeOption
	* @note 网格的配置选项
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CExGridNodeOption : public CExLodNodeOption
	{
	public:
		CExGridNodeOption();
	public:
		/**  
		  * @note 获取线、面等绘制图元的线颜色
		*/
        osg::Vec4d& lineColor();
		
		/**  
		  * @note 获取线、面等绘制图元的线宽
		*/
        int& lineWidth();

		/**  
		  * @note 获取线、面等绘制图元的线型
		*/
        unsigned int& lineStipple();

		/**  
		  * @note 网格覆盖范围（经纬度表示）
		*/
        osg::Vec4d& GridRegion();

		/**  
		  * @note 网格行数
		*/
        int& Row();

		/**  
		  * @note 网格；列数
		*/
        int& Col();

	protected:
		// 精度范围（x,y), 纬度范围(z,w)
		osg::Vec4d  m_vecRegion;

		///线、面等绘制图元的线颜色
		osg::Vec4d				m_vecLineColor;

		// 行数
		int         m_nRow; 

		// 列数
		int         m_nCol; 

		// 线、面等绘制图元的线宽
		int						m_nLineWidth;

		// 线、面等绘制图元的线型
		unsigned int			m_unLineStipple;
	};
}

namespace FeExtNode
{
	/**
	* @class CExGridNode
	* @note 网格图元对象
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CExGridNode : public CExLodNode
	{
	public:
		CExGridNode(FeUtil::CRenderContext* pRender, CExGridNodeOption* opt);

		/**  
		  * @note 设置网格图元的线颜色  
		*/
		virtual void SetLineColor( const osg::Vec4d& vecLineColor );
		
		/**  
		  * @note 获取网格图元的线颜色  
		*/
		virtual osg::Vec4d GetLineColor();

		/**  
		  * @note 设置网格图元的线宽  
		*/
		virtual void SetLineWidth( const int& nLineWidth );

		/**  
		  * @note 获取网格图元的线宽  
		*/
		virtual int GetLineWidth();

		/**  
		  * @note 设置网格图元的线型  
		*/
		virtual void SetLineStipple( const unsigned int& unLineStipple );

		/**  
		  * @note 获取网格图元的线型  
		*/
		virtual unsigned int GetLineStipple();

		/**  
		  * @note 设置网格范围（精度范围（x,y), 纬度范围(z,w)）
		*/
		virtual void SetGridRegion(const osg::Vec4& vecRegion);

		/**  
		  * @note 获取网格范围（精度范围（x,y), 纬度范围(z,w)）
		*/
		virtual osg::Vec4 GetGridRegion();

		/**  
		  * @note 设置网格行列数
		*/
		virtual void SetGridRowAndCol(int nRow, int Col);

		/**  
		  * @note 获取网格行列数
		*/
		int GetGridRow();
		int GetGridCol();

	protected:
		/**  
		  * @note 初始化图形节点 m_rpFeatureNode 对象
		  * @return 成功true, 失败false
		*/
		bool InitFeatureNode();

		/**  
		  * @note 获取矢量图形的基本构建对象
		*/
		osgEarth::Features::Feature* GetFeature();

		/**  
		  * @note 获取矢量图形的初始样式
		*/
		osgEarth::Style GetFeatureStyle();

		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExGridNodeOption* GetOption();

		/**  
		  * @note 重新构建网格
		*/
		void RebuildGrid();

	protected:
		/// 矢量图形的基本构建对象
		osg::ref_ptr<osgEarth::Features::Feature>		 m_rpFeature;     

		/// 包含图形构建对象的节点
		osg::ref_ptr<osgEarth::Annotation::FeatureNode>  m_rpFeatureNode;

		/// 地球节点
		osg::observer_ptr<osgEarth::MapNode>             m_opMapNode;

		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>        m_opRenderContext;
	};
}


#endif //FE_EXTERN_FEATURE_NODE_H
