/**************************************************************************************************
* @file FreeTreeWidget.h
* @note 自定义树控件
* @author w00040
* @data 2017-1-14
**************************************************************************************************/
#ifndef FREE_TREE_WIDGET_H
#define FREE_TREE_WIDGET_H

#include <QTreeWidget>
#include <QWheelEvent>
#include <QMenu>
#include <QMetaType>
#include <QContextMenuEvent>
#include <QAction>
#include <QKeyEvent>
#include <QDropEvent>
#include <QDragMoveEvent>

#include <mainWindow/FreeDialog.h>

namespace FreeViewer
{
	/**
	* @class CFreeTreeWidget
	* @brief 自定义树控件
	* @note 自定义树控件，实现自定义滚动条与树节点链接
	* @author w00040
	*/
	class CFreeTreeWidget : public QTreeWidget
	{
		Q_OBJECT

	public:
		/**  
		* @brief 构造函数
		*/
		CFreeTreeWidget(QWidget *parent = 0, bool isService = false);

		/**  
		* @brief 析构函数
		*/
		~CFreeTreeWidget(void);

	public:
		/** 
		* @brief 初始化数据
		*/
		void InitWidget();

	public:
		/**  
		* @brief 添加父节点
		*/
		bool AddTopTreeItem(QTreeWidgetItem* pItem);

		/**  
		* @brief 添加子节点
		*/
		bool AddTreeItem(QTreeWidgetItem* pItem, QTreeWidgetItem *pParent);

		/**  
		  * @brief 头部开始插入节点
		*/
		bool InsertFrontTreeItem(QTreeWidgetItem* pItem, QTreeWidgetItem *pParent);

		/**  
		* @brief 删除树节点(包括其所有子节点)
		*/
		void RemoveTreeItem(QTreeWidgetItem* pItem);

		/**  
		* @brief 删除树节点的所有子节点，但不删除它本身
		*/
		void RemoveTreeItemChilds(QTreeWidgetItem* pItem);

	protected:
		/**  
		* @brief 删除树节点(包括其所有子节点)
		*/
		void RemoveItemPrivate(QTreeWidgetItem* pItem);

		/**  
		* @brief 删除树节点的所有子节点，但不删除它本身
		*/
		void RemoveItemChildsPrivate(QTreeWidgetItem* pItem);

		/**  
		* @brief 更新孩子节点的选中或者未选中的状态
		*/
		void UpdateChildItemState(QTreeWidgetItem* pItem);

		/**  
		* @brief 更新父亲节点的选中或者未选中的状态
		*/
		void UpdateParentItemState(QTreeWidgetItem* pItem);

	protected:
		/**  
		* @brief 树节点右键菜单
		*/
		virtual void contextMenuEvent(QContextMenuEvent *event);

		/**  
		  * @brief 键盘按键事件
		*/
		virtual void keyPressEvent(QKeyEvent *event);

		/**  
		  * @brief 树节点拖拽移动事件
		*/
		virtual void dragMoveEvent(QDragMoveEvent *event);

		/**  
		  * @brief 树节点放置事件
		*/
		virtual void dropEvent(QDropEvent *event);

	signals:
		/**  
		* @brief 树节点右键菜单信号
		*/
		void SignalContextMenuItem(QTreeWidgetItem*, QPoint);

		/**  
		* @brief 树节点可视化改变信号
		*/
		void SignalItemsVisibleChange(QTreeWidgetItem*, bool);

		/**  
		* @brief 树节点的展开或者收起时信号
		*/
		void SignalTreeStateChanged();

		/**  
		  * @brief 树节点键盘信号
		*/
		void SignalKeyPressed(QKeyEvent*);

		/**  
		  * @brief 树节点拖拽移动信号
		*/
		void SignalDragMoveEvent(QDragMoveEvent *event);

		/**  
		  * @brief 树节点放置信号
		*/
		void SignalDropEvent(QDropEvent*);

		/**  
		  * @brief 树节点拖拽离开信号
		*/
		void SignalDragLeaveEvent(QDragLeaveEvent*);

	protected slots:
		/**  
		* @brief 点击树节点触发事件
		*/
		void SlotItemChecked(QTreeWidgetItem* pItem, int nIndex);

		private:

			bool m_bIsService; //判断是否是服务
	};
}

#endif