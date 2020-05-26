/**************************************************************************************************
* @file ExFeatureNode.h
* @note 线、面等绘制图元的基类
* @author c00005
* @data 2016-8-3
**************************************************************************************************/
#ifndef FE_EXTERN_FEATURE_NODE_H
#define FE_EXTERN_FEATURE_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExLodNode.h>

#include <osgEarthFeatures/Feature>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthAnnotation/FeatureEditing>

namespace FeUtil
{
	class CRenderContext;
}

namespace FeExtNode
{
	/**
	* @class CExFeatureNodeOption
	* @note 线、面等绘制图元的配置选项
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExFeatureNodeOption : public CExLodNodeOption
	{
	public:
		/**  
		  * @note 线、面等绘制图元节点的配置选项的构造函数
		*/
		CExFeatureNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExFeatureNodeOption();

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


	protected:
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
	* @class CExFeatureNode
	* @note 线、面等绘制图元基类，主要用于绘制线、面等绘制图元对象
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExFeatureNode : public CExLodNode
		, public FeNodeEditor::IFeEditPointEventReceiver
	{
	public:
		/**  
		  * @note 线、面等绘制图元节点基类的构造函数
		  * @param pRender [in] 渲染上下文
		  * @param opt [in] 线、面等绘制图元节点的配置属性集合
		*/
		CExFeatureNode(FeUtil::CRenderContext* pRender, CExFeatureNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		~CExFeatureNode();

	public:
		/**  
		  * @note 设置线、面图元的顶点集合  
		*/
		virtual void SetVertex(osg::Vec3dArray* pCoord);

		/**  
		  * @note 获取所有顶点
		*/
		virtual void GetVertex(osg::Vec3dArray* pCoord);

		/**  
		  * @note 向线、面图元后面中压入顶点
		*/
		virtual bool PushBackVertex(const osg::Vec3d& vecCoord);

		/**  
		  * @note 获取最后一个顶点
		*/
		virtual osg::Vec3d Back();

		/**  
		  * @note 向线、面图元前面中压入顶点  
		*/
		virtual void PushFrontVertex(const osg::Vec3d& vecCoord);

		/**  
		  * @note 获取第一个顶点 
		*/
		virtual osg::Vec3d Front();

		/**  
		  * @note 向线、面图元指定的索引处中插入顶点   
		*/
		virtual void InsertVertex(int nIndex, const osg::Vec3d& vecCoord);

		/**  
		  * @note 替换线、面图元指定的索引处的顶点   
		*/
		virtual void ReplaceVertex(int nIndex, const osg::Vec3d& vecCoord);

		/**  
		  * @note 删除指定索引的顶点 
		*/
		virtual bool DeleteVertex(int nIndex);

		/**  
		  * @note 获取顶点的数量 
		*/
		virtual int GetNumVertex();

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

		

	public:
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExFeatureNodeOption* GetOption();

	
		/** 编辑器相关接口 */
	protected:	
		/**
		  * @note 开始编辑时创建编辑点
		*/
		virtual bool StartEditImplement();

		/**
		  * @note 结束编辑时移除编辑点
		*/
		virtual bool StopEditImplement();

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
		  * @note 获取编辑点的位置
		*/
		virtual osg::Vec3d GetEditPointPositionXYZ();

		/**
		  * @note 激活编辑点
		  * @param editPoint [in] 被激活的编辑点
		*/
		void ActiveEditPoint(const FeNodeEditor::CFeEditPoint& editPoint);
		
	public:
		/**  
		  * @note 创建编辑点
		*/
		virtual osg::Group* CreateEditPoints();

		/**  
		  * @note 获取编辑点
		*/
		virtual FeNodeEditor::CFeEditPoint* GetEditPoint(unsigned int unIndex);

		/**
		  * @note 获取编辑点的位置
		*/
		virtual osgEarth::GeoPoint GetEditPointPosition();

	protected:
		/**  
		  * @note 初始化图形节点 m_rpFeatureNode 对象，由子类调用进行初始化
		          调用该函数的子类必须同时实现 GetFeature 和 GetFeatureStyle 方法
		  * @return 成功true, 失败false
		*/
		bool InitFeatureNode();

		/**  
		  * @note 获取矢量图形的基本构建对象，不同的子类实现不同的对象
		*/
		virtual osgEarth::Features::Feature* GetFeature() = 0;

		/**  
		  * @note 获取矢量图形的初始样式，子类可实现该方法返回不同的样式
		*/
		virtual osgEarth::Style GetFeatureStyle() = 0;

		/**  
		  * @note 重新构建编辑点
		*/
		virtual void RebuildEditPoints();



	protected:
		/// 矢量图形的基本构建对象
		osg::ref_ptr<osgEarth::Features::Feature>		 m_rpFeature;     

		/// 包含图形构建对象的节点
		osg::ref_ptr<osgEarth::Annotation::FeatureNode>  m_rpFeatureNode;

		/// 地球节点
		osg::observer_ptr<osgEarth::MapNode>             m_opMapNode;

		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>        m_opRenderContext;

		/// 编辑状态标记节点
		osg::ref_ptr<osg::Group>                         m_rpEditPoints;

		/// 当前编辑的顶点索引
		int                                              m_nCurEditPoint;

		/// 编辑器开始拖拽时的位置信息
		osgEarth::GeoPoint                               m_pointStartDrag;

	
	};
}


#endif //FE_EXTERN_FEATURE_NODE_H
