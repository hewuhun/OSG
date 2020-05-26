/**************************************************************************************************
* @file FeCompositeDragger.h
* @note 编辑器组合类
* @author g00034
* @data 2016-10-28
**************************************************************************************************/

#ifndef FE_EXTERN_COMPOSITE_DRAGGER_H
#define FE_EXTERN_COMPOSITE_DRAGGER_H

#include <osgManipulator/Dragger>

#define NODE_VISIBLE_MASK  0x00FF
#define NODE_UNVISBLE_MASK 0x00

namespace FeNodeEditor
{
	/**
	  * @class CFeCompositeDragger
	  * @brief 编辑器组合类
	  * @note 编辑器组合类，主要实现组合编辑器通用操作
	  * @author g00034
	*/
	class CFeCompositeDragger : public osgManipulator::CompositeDragger
	{
	public:
		/**  
		  * @note 实现父类接口，重新处理拣选事件
		*/
		virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

		/**  
		  * @note 节点是否可见
		*/
		bool IsNodeVisible(){ return (NODE_UNVISBLE_MASK != getNodeMask()); }
		void SetNodeVisible(bool bVisible){ setNodeMask(bVisible?NODE_VISIBLE_MASK:NODE_UNVISBLE_MASK); }
	};
}

#endif // FE_EXTERN_COMPOSITE_DRAGGER_H


