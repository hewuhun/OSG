/**************************************************************************************************
* @file LocalDiskTree.h
* @note 本地磁盘树
* @author g00034
* @data 2018-9-18
**************************************************************************************************/
#ifndef LOCAL_DISK_TREE_H
#define LOCAL_DISK_TREE_H

#include <QTreeWidgetItem>
#include <QFileInfo>
#include <OpenThreads/Mutex>
#include <mainWindow/FreeTreeWidget.h>

namespace FreeViewer
{
	/**
	  * @class CLocalDiskTree
	  * @brief 本地磁盘树，用于显示本地磁盘目录结构
	  * @author g00034
	*/
	class CLocalDiskTree : public QObject
	{
		Q_OBJECT

	public:
		/**  
		* @brief parentTreeWidget 树控件父窗体，strings根目录名称
		*/
		explicit CLocalDiskTree(CFreeTreeWidget* parentTreeWidget, const QStringList &strings);
		~CLocalDiskTree(void);

		/**  
		* @brief 初始化本地磁盘树
		*/
		void Initialize();

		/**  
		* @brief 重新扫描磁盘，更新本地磁盘树
		*/
		void RescanDisk();

		/**  
		* @brief 清除所有节点（不包括根节点）
		*/
		void ClearTree();

		/**  
		* @brief 获取根节点
		*/
		QTreeWidgetItem* GetTreeRootItem(){ return m_pTreeRootItem; }

		/**  
		* @brief 获取节点表示的本地全路径
		*/
		QString GetItemFullPath(QTreeWidgetItem* pItem);
		
	protected:
		/**  
		* @brief 添加子节点
		*/
		bool AddItem(QTreeWidgetItem* pItem, QTreeWidgetItem *pParentItem);
		void AddItem(const QFileInfo &fileInfo, QTreeWidgetItem* pParentItem);

		/**  
		* @brief 添加文件夹子节点
		*/
		void AddFolderChildsToTree( QString strFolderPath, QTreeWidgetItem* pParentItem );

		/**  
		* @brief 删除树节点(包括其所有子节点)
		*/
		void RemoveItem(QTreeWidgetItem* pItem);
		
		/**  
		* @brief 检查并删除无效项
		*/
		bool CheckAndRemoveInvalid(QTreeWidgetItem* pItem);

		/**  
		* @brief 检查并添加新增项
		*/
		void CheckAndInsertNew(QTreeWidgetItem* pItem);


	protected slots:
		/**  
		  * @brief 树节点展开槽函数
		*/
		void SlotItemExpanded(QTreeWidgetItem *pItem);

	private:
		CFreeTreeWidget* m_pParentTreeWidget;  // 该目录树所属的树控件对象
		QTreeWidgetItem* m_pTreeRootItem;      // 根节点
		OpenThreads::Mutex m_mutexScanDisk;    // 扫描磁盘时的互斥保护
	};

}

#endif // LOCAL_DISK_TREE_H