/**************************************************************************************************
* @file MarkPropertyBuilder.h
* @note 标记属性框创造类
* @author g00034
* @date 2017.02.20
**************************************************************************************************/

#ifndef HG_FREE_MARK_PROPERTY_BUILDER_H
#define HG_FREE_MARK_PROPERTY_BUILDER_H

#include <FeExtNode/ExternNodeVisitor.h>

namespace FreeViewer
{
	class CPropertyWidget;

    /**
    * @class CMarkPropertyBuilder
    * @brief 标记属性框创造类，负责创建标记属性对话框
    * @author g00034
    */
	class CMarkPropertyBuilder : public FeExtNode::CExternNodeVisitor
	{
	public:
		/**  
		  * @note 构造和析构函数  
		*/
		CMarkPropertyBuilder(
			FeShell::CSystemService* pSystemService, 
			FeExtNode::CExternNode* pMark,
			bool bCreate = false);

		virtual ~CMarkPropertyBuilder();

	public:
		/**  
		  * @brief 创建属性对话框
		  * @return 属性对话框
		*/
		QWidget* CreatePropertyWidget(QWidget* pParent);

		/**  
		  * @brief 根据类型进入具体类型访问器添加不同的标签页
		  * @param 参数 [in] externNode
		  * @return bool
		*/
		virtual bool VisitEnter(FeExtNode::CExComposeNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExPlaceNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExLodModelNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExLineNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExPolygonNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExBillBoardNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExOverLayNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExArcNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExEllipseNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExRectNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExSectorNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExCircleNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExTextNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExTiltModelNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExLabelNode& externNode);
		//virtual bool VisitEnter(FeExtNode::CExStraightArrowNode& externNode);
		//virtual bool VisitEnter(FeExtNode::CExAssaultNode& externNode);
		//virtual bool VisitEnter(FeExtNode::CExAttackNode& externNode);
		virtual bool VisitEnter(FeExtNode::CExParticleNode& externNode);

	protected:
		/**  
		  * @brief 创建属性框通用控件
		*/
		bool BuildCommon();

	public:
		bool				                          m_bCreate; 
		/// 属性对话框窗口
		CPropertyWidget*	                          m_pPropertyWidget;

		/// 关联的标记对象
		osg::observer_ptr<FeExtNode::CExternNode>     m_opExternNode;

		/// 系统服务对象
		osg::observer_ptr<FeShell::CSystemService>    m_opSystemService;

	};

}

#endif//HG_FREE_MARK_PROPERTY_BUILDER_H
