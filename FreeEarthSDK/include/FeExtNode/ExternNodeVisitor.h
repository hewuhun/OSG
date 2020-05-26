/**************************************************************************************************
* @file CExternNodeVisitor.h
* @note 扩展节点的遍历器基类
* @author c00005
* @data 2016-7-4
**************************************************************************************************/
#ifndef FE_EXTERN_NODE_VISITOR_H
#define FE_EXTERN_NODE_VISITOR_H

#include <FeExtNode/Export.h>

namespace FeExtNode
{
	class CExternNode;
	class CExComposeNode;
	class CExPlaceNode;
	class CExTiltModelNode;
	class CExLodModelNode;
	class CExLineNode;
	class CExPolygonNode;
	class CExBillBoardNode;
	class CExOverLayNode;
	class CExTextNode;
	class CExArcNode;
	class CExEllipseNode;
	class CExRectNode;
	class CExSectorNode;
	class CExCircleNode;
	class CExLabelNode;
	class CExPointNode;
	class CExParticleNode;
	//class CExStraightArrowNode;
	//class CExAssaultNode;
	//class CExAttackNode;

	/**
	* @class CExternNodeVisitor
	* @note 扩展节点的遍历器基类，用户可以通过继承该类实现自己的遍历器
	* @author c00005
	*/
	class FEEXTNODE_EXPORT CExternNodeVisitor
	{
	public:
		/**  
		  * @note 构造函数
		*/
		CExternNodeVisitor();

		/**  
		  * @note 析构函数
		*/
		virtual ~CExternNodeVisitor();

	public:
		/**  
		  * @brief 根据类型进入具体类型访问器添加不同的标签页
		  * @param 参数 [in] externNode
		  * @return bool
		*/
		virtual bool VisitEnter(CExternNode& externNode){return true;}

		virtual bool VisitEnter(CExComposeNode& externNode) {return true;}

		virtual bool VisitExit(CExComposeNode& externNode) {return true;}

		virtual bool VisitEnter(CExPlaceNode& externNode) {return true;}

		virtual bool VisitEnter(CExTextNode& externNode) {return true;}

		virtual bool VisitEnter(CExTiltModelNode& externNode) {return true;}

		virtual bool VisitEnter(CExLineNode& externNode) {return true;}

		virtual bool VisitEnter(CExPolygonNode& externNode) {return true;}

		virtual bool VisitEnter(CExBillBoardNode& externNode) { return true; }

		virtual bool VisitEnter(CExOverLayNode& externNode) { return true; }

		virtual bool VisitEnter(CExLodModelNode& externNode) { return true; }

		virtual bool VisitEnter(CExArcNode& externNode){ return true; }

		virtual bool VisitEnter(CExEllipseNode& externNode){ return true; }

		virtual bool VisitEnter(CExRectNode& externNode){ return true; }

		virtual bool VisitEnter(CExSectorNode& externNode){ return true; }

		virtual bool VisitEnter(CExCircleNode& externNode){ return true; }

		virtual bool VisitEnter(CExLabelNode& externNode){ return true; }

		virtual bool VisitEnter(CExParticleNode& externNode){ return true; }

		//virtual bool VisitEnter(CExStraightArrowNode& externNode){ return true; }

		//virtual bool VisitEnter(CExAssaultNode& externNode){ return true; }

		//virtual bool VisitEnter(CExAttackNode& externNode){ return true; }
	};
}

#endif //FE_EXTERN_NODE_VISITOR_H
