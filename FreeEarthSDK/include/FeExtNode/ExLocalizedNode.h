/**************************************************************************************************
* @file ExLocalizedNode.h
* @note 圆、椭圆、扇形、弧线、矩形面等绘制图元的基类。
*       这些图元的共同特征是其基本图元都继承自osgEarth::Annotation::LocalizedNode 
* @author g00034
* @data 2016-8-3
**************************************************************************************************/
#ifndef FE_EXTERN_LOCALIZED_NODE_H
#define FE_EXTERN_LOCALIZED_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExLodNode.h>

#include <osgEarthFeatures/Feature>
#include <osgEarthAnnotation/LocalizedNode>


namespace FeUtil
{
	class CRenderContext;
}

namespace FeExtNode
{
	/**
	* @class CExFeatureNodeOption
	* @note 线、面等绘制图元的配置选项
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CExLocalizedNodeOption : public CExLodNodeOption
	{
	public:
		/**  
		  * @note 线、面等绘制图元节点的配置选项的构造函数
		*/
		CExLocalizedNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExLocalizedNodeOption();

	public:
		/**  
		  * @note 获取线、面等绘制图元的线颜色
		*/
		osg::Vec4d& lineColor() { return m_vecLineColor; }
		
		/**  
		  * @note 获取线、面等绘制图元的线颜色
		*/
		osg::Vec4d& fillColor() { return m_vecFillColor; }
		
		/**  
		  * @note 获取线、面等绘制图元的线宽
		*/
		int& lineWidth() { return m_nLineWidth; }

		/**  
		  * @note 获取线、面等绘制图元的线型
		*/
		unsigned int& lineStipple() { return m_unLineStipple; }

		/**  
		  * @note 获取图元的位置
		*/
		osgEarth::GeoPoint& geoPoint() { return m_geoPoint; }
		
	protected:
		///图元的位置信息
		osgEarth::GeoPoint		m_geoPoint;

		///线、面等绘制图元的线颜色
		osg::Vec4d				m_vecLineColor;

		///线、面等绘制图元的填充颜色
		osg::Vec4d				m_vecFillColor;

		///线、面等绘制图元的线宽
		int						m_nLineWidth;

		///线、面等绘制图元的线型
		unsigned int			m_unLineStipple;
	};
}

namespace FeExtNode
{
	/**
	* @class CExLocalizedNode
	* @note 局部定位图元基类，主要用于提供图元绘制的公共接口
	* @author g00034
	*/
	class FEEXTNODE_EXPORT CExLocalizedNode : public CExLodNode
	{
	public:
		/**  
		  * @note 绘制图元节点基类的构造函数
		  * @param pRender [in] 渲染上下文
		  * @param opt [in] 绘制图元节点的配置属性集合
		*/
		CExLocalizedNode(FeUtil::CRenderContext* pRender, CExLocalizedNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		~CExLocalizedNode();
		
	public:
		/**  
		  * @note 设置线、面图元的线颜色  
		*/
		virtual void SetLineColor(const osg::Vec4d& vecLineColor);
		
		/**  
		  * @note 获取线、面图元的线颜色  
		*/
		virtual osg::Vec4d GetLineColor();

		/**  
		  * @note 设置线、面图元的填充颜色  
		*/
		virtual void SetFillColor(const osg::Vec4d& vecFillColor);

		/**  
		  * @note 获取线、面图元的填充颜色  
		*/
		virtual osg::Vec4d& GetFillColor();

		/**  
		  * @note 设置线、面图元的线宽  
		*/
		virtual void SetLineWidth(const int& nLineWidth);

		/**  
		  * @note 获取线、面图元的线宽  
		*/
		virtual int GetLineWidth();

		/**  
		  * @note 设置线、面图元的线型  
		*/
		virtual void SetLineStipple(const unsigned int& unLineStipple);

		/**  
		  * @note 获取线、面图元的线型  
		*/
		virtual unsigned int GetLineStipple();

		/**  
		  * @note 设置图元显示的位置
		  * @param geoPosition [in] 图元显示的位置，其中可以设置绝对高度和相对高度
		*/
		virtual void SetPosition(const osgEarth::GeoPoint& geoPosition);

		/**  
		  * @note 获取图元显示的位置  
		  * @return osgEarth::GeoPoint 返回的是图元显示的位置
		*/
		virtual osgEarth::GeoPoint GetPosition();

	protected:
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExLocalizedNodeOption* GetOption();

		/**
		  * @note 创建编辑工具
		*/
		virtual FeNodeEditor::CFeNodeEditTool* CreateEditorTool();

		/**
		  * @note 开始拖拽编辑
		*/
		virtual void DraggerStart();

		/**
		  * @note 拖拽编辑器矩阵变化量
		*/
		virtual void DraggerUpdateMatrix(const osg::Matrix& matrix);

	public:
		/**
		  * @note 获取编辑点位置
		*/
		virtual osg::Vec3d GetEditPointPositionXYZ();

	protected:
		/**  
		  * @note 初始化图形节点 m_rpLocalizedNode 对象，由子类调用进行初始化
		          调用该函数的子类必须同时实现 GetConcreteNode 方法
		  * @return 成功true, 失败false
		*/
		bool InitLocalizedNode();

		/**  
		  * @note 获取具体的矢量图形节点对象，子类需要实现该接口
		*/
		virtual osgEarth::Annotation::LocalizedNode* GetConcreteNode() = 0;

		/**  
		  * @note 获取矢量图形的初始样式，子类可实现该方法返回不同的样式
		*/
		virtual osgEarth::Style GetConcreteNodeStyle();
		
	protected:
		/// 包含图形构建对象的节点
		osg::ref_ptr<osgEarth::Annotation::LocalizedNode>  m_rpLocalizedNode;

		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>          m_opRenderContext;

		/// 地球节点
		osg::observer_ptr<osgEarth::MapNode>               m_opMapNode;

		/// 编辑器开始拖拽时的位置信息
		osgEarth::GeoPoint                                 m_pointStartDrag;
	};
}

#endif //FE_EXTERN_LOCALIZED_NODE_H
