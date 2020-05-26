/**************************************************************************************************
* @file ExternComposeNode.h
* @note 节点组类，相当于一个图层的概念，可以用于保存其他的不同类型的节点
* @author c00005
* @data 2016-7-14
**************************************************************************************************/
#ifndef FE_EXNODE_COMPOSE_NODE_H
#define FE_EXNODE_COMPOSE_NODE_H

#include <FeExtNode/Export.h>
#include <FeExtNode/ExLodNode.h>

namespace FeExtNode
{
	/**
	* @class CExternNodeOption
	* @note 扩展节点的配置选项,其中包含了名称、描述、视点以及是否可见
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExComposeNodeOption : public CExLodNodeOption
	{
	public:
		///定义一个用于保存该节点组下的所有节点
        typedef std::vector<osg::ref_ptr<CExternNode> > VecExternNodes;

	public:
		/**  
		  * @note 构造函数，无参数，使用默认值
		*/
		CExComposeNodeOption();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExComposeNodeOption();

	public:
		/**  
		  * @note 获取节点的名称
		*/
		VecExternNodes& childrens() { return m_vecChildren; }

	protected:
		///ComposeNode节点下保存的孩子序列
		VecExternNodes    m_vecChildren;   
	};
}

namespace FeExtNode
{
	/**
	* @class CExComposeNode
	* @note 节点组类，相当于一个图层的概念，可以用于保存其他的不同类型的节点
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExComposeNode : public CExLodNode
	{

	public:
		/**  
		  * @note 构造函数
		  * @param opt [in] 节点组的属性配置集合
		*/
		CExComposeNode(CExLodNodeOption* opt);
		
		/**  
		  * @note 析构函数
		*/
		virtual ~CExComposeNode();

	public:
		/**  
		  * @note 判断是否存在该节点
		  * @param pNode [in] 需要被判断的节点指针
		*/
		virtual bool ExistNode(CExternNode* pNode);

		/**  
		  * @note 向当前组节点中插入节点
		  * @param pNode [in] 被插入的节点指针
		*/
		virtual bool InsertNode(CExternNode* pNode);

		/**  
		  * @note 从组节点中移除的节点
		  * @param pNode [in] 需要被移除的节点指针
		*/
		virtual bool RemoveNode(CExternNode* pNode);

		/**  
		  * @note 从组节点中获取指定ID的节点
		  * @return CExternNode* 获取到的节点的指针，如果没有找到，则返回为NULL
		*/
		virtual CExternNode* GetNode(const FEID& nId);

		/**
		* @note 从组节点中获取所有的节点
		*/
		virtual CExComposeNodeOption::VecExternNodes GetChildren();

		/**  
		  * @note 清空组节点下的所有节点
		*/
		virtual void Clear();
			
	public:
		/**  
		  * @note 继承自osg::Group,用于在每一帧中遍历节点，
		  *		  进而判断节点是否在可视视距范围内，从而控制显示或者隐藏
		*/
		virtual void traverse(osg::NodeVisitor& nv); 

		/**
		* @note 是否可以作为一个节点组
		*/
		virtual CExComposeNode* AsComposeNode();

		/**  
		  * @note 接受节点遍历器，可以接受用户自定义的各种遍历，比如编辑操作、保存操作等功能的遍历器实现
		*/
		virtual void Accept(CExternNodeVisitor& nv);

	protected:
		/**  
		  * @note 重写父类的方法，获取当前节点的Options信息
		*/
		virtual CExComposeNodeOption* GetOption();

		/**  
		  * @note 更新节点的掩码，用于显示或者接受事件状态改变时调用
		*/
		virtual void UpdateVisibleAndEvent();
	};

}

#endif //EXNODE_COMPOSE_NODE_H
