/**************************************************************************************************
* @file MarkPluginInterface.h
* @note 场景标记界面管理类
* @author g00034
* @data 2017-02-09
**************************************************************************************************/
#ifndef MARK_PLUGIN_INTERFACE_H
#define MARK_PLUGIN_INTERFACE_H 1

#include <QObject>
#include <QVector>

#include <FeManager/FreeMarkSys.h>

#include <mark/MarkMenuMgr.h>
#include <mark/MarkEventHandler.h>
#include <mark/MarkVisitProcessor.h>

#include <mainWindow/UIObserver.h>
#include <mainWindow/Free3DDockTreeWidget.h>

#include <mark/MarkSdkEventCapture.h>

namespace FreeViewer
{
    /**
      * @class CMarkPluginInterface
      * @note 管理场景标记的界面
      * @author g00034
    */
    class CMarkPluginInterface : public CUIObserver
    {
		Q_OBJECT
    public:
		/**  
		  * @note 构造函数  
		  * @param pMainWindow [in] 主窗口
		*/
		CMarkPluginInterface(CFreeMainWindow* pMainWindow);

		/**  
		  * @note 析构函数
		*/
        ~CMarkPluginInterface();

		/**  
		  * @brief 获取主界面
		*/
		CFreeMainWindow* GetMainWindow();

	protected:
		/**
		  *brief 初始化控件
		*/
		void InitWidget();
		
		/**
		  *brief 初始化环境，主要构件界面以及注册一些功能
		*/
		void BuildContext();

	public:
		/**  
		  * @brief 开始绘制标记，目前只供 CMarkEventHandler 调用
		  * @note 调用该函数意味着标记绘制已经开始  
		*/
		void StartDrawMark();

		/**  
		  * @brief 标记绘制成功，结束标记绘制，显示属性框
		*/
		void SuccessToDraw();

		/**  
		  * @brief 标记绘制失败，结束标记绘制，不显示属性框
		*/
		void FailedToDraw();

		/**  
		  * @brief 显示指定标记的属性对话框
		*/
		void ShowMarkPropertyWidget(FeExtNode::CExternNode* pMarkNode, bool bCreate);

		/**  
		  * @brief 判断当前是否有标记正在绘制或正在编辑
		*/
		bool IsDrawingOrEditing();

		/**  
		  * @brief 获取标记遍历处理器
		*/
		CMarkVisitProcessor& GetMarkProcessor();

		/**  
		  * @brief 开始标记事件捕获
		*/
		void StartEventCapture();

		/**  
		  * @brief 停止标记事件捕获
		*/
		void StopEventCapture();

		/**  
		  * @brief 获取场景管理系统
		*/
		FeManager::CFreeMarkSys* GetMarkSys();

		/**  
		  * @brief 获取树形控件
		*/
		CFree3DDockTreeWidget* GetTreeDockWidget();

		/**  
		  * @brief 设置当前激活的标记节点
		*/
		void SetActiveMarkNode(FeExtNode::CExternNode* pNode);

		/**  
		  * @brief 清除当前标记
		*/
		void DeleteCurrentMark();

	private:
		/**
		  *brief 初始化树形结构控件
		*/
		void InitMarkTree();

		/**
		  *brief 初始化菜单
		*/
		void InitMarkMenu();

		/**
		  *brief 添加标记到树形控件
		*/
		void AddTreeItem(QTreeWidgetItem* pParentItem, FeExtNode::CExternNode* pExternNode);
	
		/**  
		  * @brief 通过树节点获取与之关联的标记节点
		*/
		FeExtNode::CExternNode* GetMarkByItem(QTreeWidgetItem*);
		
		/**  
		  * @brief 获取当前树形节点所属组节点
		*/
		QTreeWidgetItem* GetCurrentGroupItem();

		/**  
		  * @brief 重置所有状态，所有操作结束后调用
		*/
		void ResetAllState();

		/**  
		  * @brief 从标记系统中删除指定节点，包括其所有子节点
		*/
		void DeleteNodeFromMarkSys(FeExtNode::CExternNode* pNode, FeExtNode::CExternNode* pParentNode);



	public slots:
		/// 属性对话框用户选择结果槽函数
		void SlotPropertyOK(FeExtNode::CExternNode* pNode);
		void SlotPropertyCancel(FeExtNode::CExternNode* pNode);

		/// 监听器卸载槽函数
		void SlotRemoveEventListener(FeExtNode::CExternNode* pNode);

		/// 响应树形控件的相关槽函数
		void SlotContextMenuItem(QTreeWidgetItem*, QPoint);
		void SlotItemsVisibleChange(QTreeWidgetItem*, bool);
		void SlotItemDBClicked(QTreeWidgetItem*);

		/// 右键菜单相关槽函数
		void SlotMenuOpenPropertyWidget();
		void SlotMenuDeleteMark();
		void SlotMenuClearMark();

		/// SDK事件响应
		void SlotNodeStateChanged(unsigned int);

	private:
		/// 场景标记管理系统
		osg::ref_ptr<FeManager::CFreeMarkSys> m_rpMarkSys;

		/// 客户端场景标记事件处理对象
		osg::ref_ptr<CMarkEventHandler>       m_rpMarkHandler;

		/// 当前标记节点
		osg::ref_ptr<FeExtNode::CExternNode>  m_rpCurMarkNode;

		/// 是否正在创建标记
		bool                                  m_bCreatingMark;

		/// 是否正在显示标记属性框
		bool                                  m_bPropertyWidgetShow;

		/// 树形控件
		CFree3DDockTreeWidget*                m_p3DTreeWidget;

		///标记节点的根节点
		QTreeWidgetItem*				      m_pMarkTreeItemRoot;

		///当前标记节点中激活的节点
		QTreeWidgetItem*					  m_pActiveItem;

		/// 标记遍历处理器
		CMarkVisitProcessor                   m_markProcessor;

		/// 标记菜单管理器
		CMarkMenuMgr                          m_markMenuMgr;

		///当前的属性对话框
		QWidget*							  m_pCurrentWidget;

		///SDK事件监听器
		osg::ref_ptr<CMarkEventListener>      m_rpEventListener;
	};

}
#endif // MARK_PLUGIN_INTERFACE_H
