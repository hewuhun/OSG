/**************************************************************************************************
* @file FreeServiceDockTree.h
* @note 服务系统管理树
* @author w00040
* @data 2018-7-30
**************************************************************************************************/
#ifndef FREE_SERVICE_DOCK_TREE_H
#define FREE_SERVICE_DOCK_TREE_H 1

#include <QWidget>
#include <QTreeWidgetItem>
#include <QContextMenuEvent>

#include <mainWindow/FreeDockWidget.h>
#include <mainWindow/FreeTreeWidget.h>
#include <mainWindow/FreeServiceButton.h>

#include <FeServiceProvider/ServiceProvider.h>

//声明Qt类型
Q_DECLARE_METATYPE(FeServiceProvider::ServiceProvider *);
Q_DECLARE_METATYPE(FeServiceProvider::ServiceLayer *);

namespace FreeViewer
{
	///QTreeWidgetItem所代表的具体类型，保存在Data中
	enum EServiceItemType
	{
		// 服务系统
		E_SERVICE = 0,

		// 本地服务
		E_LOCAL_SERVICE,
		E_LOCAL_SERVICE_ROOT,//根节点
		E_LOCAL_SERVICE_FOLDER,//叶节点
		E_LOCAL_SERVICE_IMAGE_NODE,//子节点
		E_LOCAL_SERVICE_MODEL_NODE,//子节点

		// OGC服务
		E_OGC_SERVICE,
		E_OGC_SERVICE_ROOT,//根节点
		E_WMS_SERVICE_ROOT,//根节点
		E_WMTS_SERVICE_ROOT,//根节点
		E_WFS_SERVICE_ROOT,//根节点
		E_WCS_SERVICE_ROOT,//根节点
		E_OGC_SERVICE_FOLDER,//叶节点
		E_WMS_SERVICE_NODE,//子节点
		E_WMTS_SERVICE_NODE,//子节点
		E_WFS_SERVICE_NODE,//子节点
		E_WCS_SERVICE_NODE,//子节点

		// Tile服务
		E_TILE_SERVICE,
		E_TILE_SERVICE_ROOT,//根节点
		E_TMS_SERVICE_ROOT,//根节点
		E_XYZ_SERVICE_ROOT,//根节点
		E_TMS_SERVICE_NODE,//子节点
		E_XYZ_SERVICE_NODE,//子节点

		// 数据集服务
		E_DATA_SERVICE,
		E_DATA_SERVICE_ROOT,//根节点
		E_DATA_SERVICE_NODE//子节点
	};

	class CUIObserver;

	/**
	  * @class CFreeServiceDockTree
	  * @brief 服务系统管理树的Dock窗口
	  * @note 负责分发不同树节点的信号和槽
	  * @author w00040
	*/
	class CFreeServiceDockTree : public CFreeDockWidget
	{
		Q_OBJECT

	public:
		/**  
		 * @brief 构造函数
		 */
		CFreeServiceDockTree(QString titleText, QWidget *parent = 0);

		/**  
		 * @brief 析构函数
		 */
		~CFreeServiceDockTree();

		/**  
		  * @brief 初始化树控件
		*/
		void InitWidget();

		/**  
		  * @brief 获取到树控件
		*/
		CFreeTreeWidget* GetTreeWidget();
		
		/**  
		  * @brief 保存节点类型对应的系统，后面会根据系统类型调用系统的槽函数
		*/
		void PushBackSystem(EServiceItemType type, CUIObserver* pSystem);

		/**  
		  * @brief 树节点拖拽
		*/
		void DragItem();

	protected slots:
		/**  
		  * @brief 右键菜单槽函数
		*/
		void SlotContextMenuItem(QTreeWidgetItem*, QPoint);

		/**  
		  * @brief 树节点点击槽函数
		*/
		void SlotItemClicked(QTreeWidgetItem *pItem, int nColumn);

		/**  
		  * @brief 树节点双击槽函数
		*/
		void SlotItemDoubleClicked(QTreeWidgetItem *pItem, int nColumn);

		/**  
		  * @brief 树节点展开槽函数
		*/
		void SlotItemExpanded(QTreeWidgetItem *pItem);

		/**  
		  * @brief 树节点改变槽函数
		*/
		void SlotItemVisibleChange(QTreeWidgetItem *pItem, bool bState);

		/**  
		  * @brief 停靠窗按钮点击槽函数
		*/
		void SlotBtnClicked(EServiceButtonType type);

		/**  
		  * @brief 树节点键盘事件槽函数
		*/
		void SlotKeyPressed(QKeyEvent *event);

		/**  
		  * @brief 树节点拖拽移动槽函数
		*/
		void SlotDragMoveEvent(QDragMoveEvent *event);

	signals:
		/**  
		  * @brief 树节点拖拽信号
		*/
		void SignalDragItem(bool);

	protected:
		// 保存类型和服务的map对
		typedef std::map<EServiceItemType, CUIObserver*> TypeServiceMap;
		TypeServiceMap					m_mapTypeService;

		// 自定义树控件
		CFreeTreeWidget*				m_pServiceTreeWidget;

		// 停靠窗按钮
		CFreeServiceButton*				m_pServiceButton;
	};
}


#endif
