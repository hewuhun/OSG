/**************************************************************************************************
* @file ExLineNode.h
* @note 线图元的基类
* @author c00005
* @data 2016-8-3
**************************************************************************************************/
#ifndef FE_EXTERN_LINE_NODE_H
#define FE_EXTERN_LINE_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExFeatureNode.h>

namespace FeExtNode
{
	/**
	* @class CExLineNodeOption
	* @note 线图元的配置选项
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExLineNodeOption : public CExFeatureNodeOption
	{
	public:
		/**  
		  * @note 线图元节点的配置选项的构造函数
		*/
		CExLineNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExLineNodeOption();
	};
}

namespace FeExtNode
{
	/**
	* @class CExLineNode
	* @note 线图元基类，主要用于绘制线图元对象
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExLineNode : public CExFeatureNode
	{
	public:
		/**  
		  * @note 线图元节点基类的构造函数
		  * @param pRender [in] 渲染上下文
		  * @param opt [in] 线节点的配置属性集合
		*/
		CExLineNode(FeUtil::CRenderContext* pRender, CExLineNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		~CExLineNode();
	
	public:
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExLineNodeOption* GetOption();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

		/**  
		  * @note 线不提供填充色功能
		*/
		virtual void SetFillColor(const osg::Vec4d& vecFillColor){}

		/**  
		  * @brief 检测顶点（线的顶点数必须大于等于2）
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

#endif //FE_EXTERN_LINE_NODE_H
