/**************************************************************************************************
* @file ExPointNode.h
* @note 点图元的基类
* @author c00005
* @data 2016-8-3
**************************************************************************************************/
#ifndef FE_EXTERN_POINT_NODE_H
#define FE_EXTERN_POINT_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExLodNode.h>

#include <osgEarth/GeoData>
#include <osgEarthAnnotation/PlaceNode>
#include <osgEarthSymbology/TextSymbol>

namespace FeUtil
{
	class CRenderContext;
}

namespace FeExtNode
{
	/**
	* @class CExPointNodeOption
	* @note 点图元的配置选项
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExPointNodeOption : public CExLodNodeOption
	{
	public:
		/**  
		  * @note 点图元节点的配置选项的构造函数
		*/
		CExPointNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExPointNodeOption();

	public:
		/**  
		  * @note 获取点图元的位置
		*/
		osgEarth::GeoPoint& geoPoint() { return m_geoPoint; }
		
	protected:
		///点图元的位置信息
		osgEarth::GeoPoint			m_geoPoint;
	};
}

namespace FeExtNode
{
	/**
	* @class CExPointNode
	* @note 点图元节点基类，主要用于绘制点图元对象
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExPointNode : public CExLodNode
	{
	public:
		/**  
		  * @note 点图元节点基类的构造函数
		   * @param pRender [in] 渲染上下文
		  * @param opt [in] Lod节点的配置属性集合
		*/
		CExPointNode(FeUtil::CRenderContext* pRender, CExPointNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		~CExPointNode();

	public:
		/**  
		  * @note 设置点图元节点的内容
		  * @param strName [in] 点图元节点的内容
		*/
		virtual void SetName(const std::string& strName);

		/**  
		  * @note 设置点图元节点显示的位置
		  * @param geoPosition [in] 点图元节点显示的位置，其中可以设置绝对高度和相对高度
		*/
		virtual void SetPosition(const osgEarth::GeoPoint& geoPosition);

		/**  
		  * @note 获取点图元节点显示的位置  
		  * @return osgEarth::GeoPoint 返回的是点图元节点显示的位置
		*/
		virtual osgEarth::GeoPoint GetPosition();

	public: 
		/** 编辑器相关接口 */
		
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

		/**
		  * @note 获取编辑点位置
		*/
		virtual osg::Vec3d GetEditPointPositionXYZ();

	public:
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExPointNodeOption* GetOption();

		/**  
		  * @note 获取地球节点，目前主要用于编辑器
		*/
		virtual osgEarth::MapNode* GetMapNode();

	protected:
		///地球节点
		osg::observer_ptr<osgEarth::MapNode>	   m_opMapNode;

		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext>  m_opRenderContext;

		/// 编辑器开始拖拽时的位置信息
		osgEarth::GeoPoint                         m_pointStartDrag;
	};
}



#endif //FE_EXTERN_POINT_NODE_H
