/**************************************************************************************************
* @file ServicePluginInterface.h
* @note 服务管理接口
* @author w00040
* @data 2018-7-23
**************************************************************************************************/
#ifndef SERVICE_PLUGIN_INTERFACE_H
#define SERVICE_PLUGIN_INTERFACE_H 1

#include <mainWindow/UIObserver.h>
#include <mainWindow/FreeServiceDockTree.h>
//#include <mainWindow/FreeWarningWidget.h>
#include <service/OGCServiceWidget.h>
#include <service/TileServiceWidget.h>
#include <service/DataServiceWidget.h>
#include <service/OGCServicePropertyWidget.h>
#include <service/OGCLayerPropertyWidget.h>
#include <service/ServiceLayerPropertyWidget.h>
#include <service/LocalDiskTree.h>

#include <FeServiceProvider/ServiceProvider.h>
#include <FeServiceProvider/ServiceManager.h>

#include <QTreeWidgetItem>
#include <QFileInfoList>
#include <QFileInfo>
#include <QHash>

#include <layer/CustomLayerTree.h>
namespace FreeViewer
{
	/**
	  * @class CServicePluginInterface
	  * @brief 服务管理接口
	  * @note 提供服务相关功能管理
	  * @author w00040
	*/
	class CServicePluginInterface : public CUIObserver
	{
		Q_OBJECT

	public:
		/**  
		  * @note 构造函数
		*/
        CServicePluginInterface(CFreeMainWindow *pMainWindow);
		
		/**  
		  * @note 析构函数
		*/
		~CServicePluginInterface();

		/**  
		  * @brief 初始化窗体
		*/
		void InitWidget();

		/**  
		  * @brief 初始化上下文
		*/
		void InitContext();

		/**  
		  * @brief 设置自定义图层树
		*/
		void SetCustomLayerTree(CCustomLayerTree * pCustomLayerTree);

		/**  
		  * @brief 初始化展开树节点
		*/
		void InitExpandTree();

		/**  
		  * @brief 保存服务
		*/
		void SaveData();

		/**  
		  * @brief 加载服务
		*/
		void LoadData();

		/**
		  * @brief 磁盘扫描，为空扫描磁盘
		*/
		void ScanDrivers(QString strPath = "");

		/**
			* @brief 添加节点
		*/
		void AddItem(QFileInfo &fileInfo);

		/**  
		  * @brief 刷新本地服务
		*/
		void UpdateLocalService();

		/**  
		  * @brief 刷新OGC服务
		*/
		void UpdateOGCService();

		/**  
		  * @brief 树节点右键菜单
		*/
		void ContextMenuItem(QTreeWidgetItem* pItem, QPoint point);

		/**  
		  * @brief 树节点点击
		*/
		void ItemClicked(QTreeWidgetItem *item);

		/**  
		  * @brief 树节点双击
		*/
		void ItemDoubleClicked(QTreeWidgetItem *item);

		/**  
		  * @brief 编辑重命名节点
		*/
		void EditRenameItem(QTreeWidgetItem *pItem, bool bState);

		/**  
		  * @brief 添加子服务节点
		*/
		void AddItemService(FeServiceProvider::ServiceProvider *pServiceProvider, QTreeWidgetItem *pParent, EServiceItemType type);

		/**  
		  * @brief 停靠窗按钮点击
		*/
		void BtnClicked(EServiceButtonType type);

		/**  
		  * @brief 设置当前激活的节点
		*/
		void SetCurrentItem(QTreeWidgetItem *pItem);

		/**  
		  * @brief 获取当前激活的节点
		*/
		QTreeWidgetItem* GetCurrentItem();

		/**  
		  * @brief R键属性
		*/
		void RKeyPressed();

		/**  
		  * @brief F2重命名
		*/
		void F2KeyPressed();

		/**  
		  * @brief Delete键删除
		*/
		void DeleteKeyPressed();

	protected slots:
		/**  
		  * @brief 添加OGC标准服务槽函数
		*/
		void SlotAddOGCService();

		/**  
		  * @brief 添加Tile服务槽函数
		*/
		void SlotAddTileService();

		/**  
		  * @brief 添加数据集槽函数
		*/
		void SlotAddDataService();

		/**  
		  * @brief 添加子服务到服务管理树槽函数
		*/
		void SlotAddServiceToTree(FeServiceProvider::ServiceProvider *pServiceProvider);

		/**  
		  * @brief 属性槽函数
		*/
		void SlotProperty();

		/**  
		  * @brief 删除槽函数
		*/
		void SlotDelete();

		/**  
		  * @brief 添加服务图层到场景槽函数
		*/
		void SlotAddLayer();

		/**  
		  * @brief 重命名槽函数
		*/
		void SlotRename();

		/**  
		  * @brief 刷新槽函数
		*/
		void SlotUpdate();

		/**  
		  * @brief 添加数据的服务树，当前选中的节点文本槽函数
		*/
		void SlotServiceCurrentItem(QString strText);

		/**  
		  * @brief 添加数据图层树节点双击槽函数
		*/
		void SlotItemDoubleClicked(EServiceItemType type);

		/**  
		  * @brief 重置当前激活节点为空槽函数
		*/
		void SlotResetCurrentItem();

	signals:
		/**  
		  * @brief 发送添加服务图层到场景信号
		*/
		void SignalAddLayerItem(QTreeWidgetItem*);

		/**  
		  * @brief 发送添加数据当前图层的树节点信号
		*/
		void SignalLayerCurrentItem(QTreeWidgetItem*);

		/**  
		  * @brief 服务添加完成信号
		*/
		void SignalAddServiceFinished();

	private:
		// 添加OGC标准服务
		QAction*							m_pActionOGC;

		// 添加Tile服务
		QAction*							m_pActionTile;

		// 添加数据集
		QAction*							m_pActionData;

		// 属性
		QAction*							m_pActionProperty;

		// 删除
		QAction*							m_pActionDelete;

		// 重命名
		QAction*							m_pActionRename;

		// 右键添加图层到场景
		QAction*							m_pActionAddLayer;

		// 右键刷新
		QAction*							m_pActionUpdate;

		// 服务管理树
		CFreeServiceDockTree*				m_pServiceDockTree;

		// 本地磁盘服务根节点
		QTreeWidgetItem*					m_pTreeRootItemLocal;

		// OGC标准服务界面
		COGCServiceWidget*					m_pOGCServiceWidget;

		// OGC标准服务根节点
		QTreeWidgetItem*					m_pTreeRootItemOGC;

		// WMS服务根节点
		QTreeWidgetItem*					m_pTreeRootItemWMS;

		// WMTS服务根节点
		QTreeWidgetItem*					m_pTreeRootItemWMTS;

		// WFS服务根节点
		QTreeWidgetItem*					m_pTreeRootItemWFS;

		// WCS服务根节点
		QTreeWidgetItem*					m_pTreeRootItemWCS;

		// Tile服务界面
		CTileServiceWidget*					m_pTileServiceWidget;

		// Tile服务根节点
		QTreeWidgetItem*					m_pTreeRootItemTile;

		// TMS服务根节点
		QTreeWidgetItem*					m_pTreeRootItemTMS;

		// XYZ服务根节点
		QTreeWidgetItem*					m_pTreeRootItemXYZ;

		// 数据集服务界面
		CDataServiceWidget*					m_pDataServiceWidget;

		// 数据集服务根节点
		QTreeWidgetItem*					m_pTreeRootItemData;

		// 当前激活的树节点
		QTreeWidgetItem*					m_pItemCurrent;

		// OGC服务属性界面
		COGCServicePropertyWidget*			m_pOGCServicePropertyWidget;

		// OGC图层属性界面
		COGCLayerPropertyWidget*			m_pOGCLayerPropertyWidget;

		// Tile和数据集服务图层属性
		CServiceLayerPropertyWidget*		m_pServiceLayerPropertyWidget;

		// 服务序列化和反序列化
		FeServiceProvider::ServiceManager*	m_pServiceManager;

		// 存储服务根节点
		QVector<QTreeWidgetItem*>			m_vecService;

		// Hash列表
		QHash<QString, QTreeWidgetItem*>    m_hashPathItem;

		// 双击展开的树节点
		QVector<QTreeWidgetItem*>           m_vecDoubleItems;

		// 保存要删除的树节点
		QVector<QTreeWidgetItem*>			m_vecDeleteItems;

		// 树节点映射表，1表示存在，0表示不存在,更新时将会删除
		QMap<QTreeWidgetItem*, int>         m_mapAddItems;

		// 更新标识
		bool                                m_bUpdateFlag;

		// 保存编辑重命名前的节点名
		QString								m_strEditItemName;

		//// 警告对话框
		//CFreeWarningWidget*					m_pWarningWidget;

		// 当前属性窗打开状态
		bool								m_bIsShowProperty;

		///item 编辑状态
		bool								m_bItemEditState;

		// 本地磁盘树对象
		CLocalDiskTree*                     m_pLocalDiskTree;

		CCustomLayerTree *                  m_pCustomLayerTree;
	};
}

#endif 
