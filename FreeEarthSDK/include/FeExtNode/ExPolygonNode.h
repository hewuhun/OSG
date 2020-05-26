/**************************************************************************************************
* @file ExPolygonNode.h
* @note 多边形图元的基类
* @author c00005
* @data 2016-8-3
**************************************************************************************************/
#ifndef FE_EXTERN_POLYGON_NODE_H
#define FE_EXTERN_POLYGON_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExFeatureNode.h>

namespace FeExtNode
{
	/**
	* @class CExPolygonNodeOption
	* @note 多边形图元的配置选项
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExPolygonNodeOption : public CExFeatureNodeOption
	{
	public:
		/**  
		  * @note 多边形图元节点的配置选项的构造函数
		*/
		CExPolygonNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExPolygonNodeOption();
	};
}

namespace FeExtNode
{
	/**
	* @class CExPolygonNode
	* @note 多边形图元基类，主要用于绘制多边形图元对象
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExPolygonNode : public CExFeatureNode
	{
	public:
		/**  
		  * @note 多边形图元节点基类的构造函数
		  * @param pRender [in] 渲染上下文
		  * @param opt [in] 多边形图元节点的配置属性集合
		*/
		CExPolygonNode(FeUtil::CRenderContext* pRender, CExPolygonNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		~CExPolygonNode();
	
	public:
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExPolygonNodeOption* GetOption();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**  
		  * @brief 检测顶点（面的顶点数必须大于等于3）
		*/
		virtual bool CheckedVertex();

	private:
		/**  
		  * @note 获取矢量图形的基本构建对象，不同的子类实现不同的对象
		*/
		virtual osgEarth::Features::Feature* GetFeature();

		/**  
		  * @note 获取矢量图形的初始样式，子类可实现该方法返回不同的样式
		*/
		virtual osgEarth::Style GetFeatureStyle();
	
	};
}

#endif //FE_EXTERN_POLYGON_NODE_H
