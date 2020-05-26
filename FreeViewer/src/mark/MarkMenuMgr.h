/**************************************************************************************************
* @file MarkMenuMgr.h
* @note 场景标记菜单管理类
* @author g00034
* @data 2017-02-17
**************************************************************************************************/
#ifndef MARK_MENU_MGR_H
#define MARK_MENU_MGR_H 1

#include <QObject>
#include <FeExtNode/ExternNode.h>

class QMenu;
class QAction;

namespace FreeViewer
{
	class CMarkPluginInterface;

	/**
	  * @class CMarkMenuMgr
	  * @brief 场景标记菜单管理类
	  * @note 集中管理菜单相关逻辑和操作
	  * @author g00034
	*/
	class CMarkMenuMgr : public QObject
	{
		Q_OBJECT

	public:
		CMarkMenuMgr(CMarkPluginInterface* pMarkPlugin);
			
		/**  
		  * @brief 初始化菜单
		*/
		void InitMenuAndActions();

		/**  
		  * @brief 为指定图元创建右键菜单
		*/
		QMenu* CreatePopMenu(FeExtNode::CExternNode* pExternNode, QWidget* pMenuParent);

		/**  
		  * @brief 为图元根节点创建右键菜单
		*/
		QMenu* CreateRootPopMenu(FeExtNode::CExternNode* pExternNode, QWidget* pMenuParent);

		/**  
		  * @brief 重置菜单状态
		*/
		void ResetMenu() { SetMutex(NULL); }

		/**  
		  * @brief 菜单互斥操作
		*/
		void SetMutex( QAction* pCurrentAction );

	private:
		/**  
		  * @brief 创建添加图元菜单
		*/
		QMenu* CreateAddMenu(QWidget* pMenuParent);

		/**  
		  * @brief 对标记的激活进行一致处理
		  * @note 所有标记的激活事件逻辑在此函数中处理
		  * @param pNode [in] 当前激活的标记节点
		  * @param pAction [in] 关联的 QAction 对象
		  * @param bAction [in] 是否激活
		  * @return 无返回值
		*/
		void HandleMarkAction(osg::ref_ptr<FeExtNode::CExternNode> pNode, QAction* pAction, bool bAction);

	signals:
		/// 右键菜单相关槽函数
		void SignalOpenPropertyWidget();
		void SignalDeleteMark();
		void SignalClearMark();

	private slots:
		/// 标记对应的按钮点击事件槽函数
		void SlotAddFolder(bool bAction);
		void SlotAddPoint(bool bAction);
		void SlotAddLine(bool bAction);
		void SlotAddPolygon(bool bAction);
		//void SlotAddStraight(bool bAction);
		//void SlotAddAssault(bool bAction);
		//void SlotAddAttack(bool bAction);
		void SlotAddArc(bool bAction);
		void SlotAddEllipse(bool bAction);
		void SlotAddRect(bool bAction);
		void SlotAddSector(bool bAction);
		void SlotAddCircle(bool bAction);
		void SlotAddText(bool bAction);
		void SlotAddLabel(bool bAction);
		void SlotAddModel(bool bAction);
		void SlotAddTiltModel(bool bAction);
		void SlotAddBillboard(bool bAction);
		void SlotAddOverlay(bool bAction);
		void SlotAddParticle(bool bAction);
		
	private:
		/// 标记对应的 QAction
		QAction*                              m_pFolderAction;
		QAction*                              m_pPointAction;
		QAction*                              m_pLineAction;
		QAction*                              m_pPolygonAction;
		//QAction*                              m_pStraightAction;
		//QAction*                              m_pAssaultAction;
		//QAction*                              m_pAttackAction;
		QAction*                              m_pArcAction;
		QAction*                              m_pEllipseAction;
		QAction*                              m_pRectAction;
		QAction*                              m_pSectorAction;
		QAction*                              m_pCircleAction;
		QAction*                              m_pTextAction;
		QAction*                              m_pLabelAction;
		QAction*                              m_pModelAction;
		QAction*                              m_pTiltModelAction;
		QAction*                              m_pBillboardAction;
		QAction*                              m_pOverlayAction;
		QAction*                              m_pParticleAction;

		/// 当前活动的Action
		QAction*	                          m_pActiveAction;  

		/// 右键菜单相关 Action
		QAction*	                          m_pOpenPropertyAction;    
		QAction*                              m_pDeleteMarkAction;
		QAction*                              m_pClearMarkAction;

	private:
		CMarkPluginInterface*                    m_pMarkPlugin;
	};
}



#endif // MARK_MENU_MGR_H
