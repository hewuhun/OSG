/**************************************************************************************************
* @file ExLabelBindingNodeCallback.h
* @note 标牌绑定节点更新回调类，用于更新标牌位置
* @author g00034
* @data 2016-9-9
**************************************************************************************************/
#ifndef EX_LABEL_BINDING_NODE_H
#define EX_LABEL_BINDING_NODE_H

#include <FeExtNode/Export.h>

namespace FeExtNode
{
	class CExLabelNode;
	
	/**
	  * @class CExLabelBindingNodeCallback
	  * @note 标牌绑定节点更新回调类，用于更新标牌位置
	  * @author g00034
	*/
	class FEEXTNODE_EXPORT CExLabelBindingNodeCallback : public osg::NodeCallback
	{
	public:
		/**  
		  * @note 构造函数
		  * @param pRender [in] 当前渲染上下文
		*/
		CExLabelBindingNodeCallback(FeUtil::CRenderContext* pRender);

		/**  
		  * @note 析构函数
		*/
		~CExLabelBindingNodeCallback(void);

		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

		/**  
		  * @note 增加标牌绑定节点，一个节点可以绑定多个标牌，但一个标牌只能绑定一个节点  
		  * @param pLabel [in]  标牌节点
		  * @param pBindNode [in] 绑定节点
		  * @return 是否添加成功
		*/
		bool AddLabelBindNode(CExLabelNode* pLabel, osg::Node* pBindNode);

		/**  
		  * @note 移除指定绑定节点
		  * @param pLabel [in]  绑定节点
		  * @return 是否移除成功
		*/
		bool RemoveBindNode(osg::Node* pBindNode);
	

	private:
		/// 要绑定的节点
		osg::observer_ptr<osg::Node>    m_opBindingNode;

		/// 绑定到节点的标牌
		osg::observer_ptr<CExLabelNode> m_opLabelNode;  

		/// 当前渲染上下文
		osg::observer_ptr<FeUtil::CRenderContext> m_opRenderContext;

	};

}

#endif // EX_LABEL_BINDING_NODE_H

