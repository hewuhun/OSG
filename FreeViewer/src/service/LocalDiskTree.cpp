#include <QTreeWidgetItem>
#include <QFileInfoList>
#include <QFileInfo>
#include <QDir>
#include <QVariant>
#include <OpenThreads/ScopedLock>

#include <mainWindow/FreeServiceDockTree.h>
#include "LocalDiskTree.h"

#include <iostream>

namespace FreeViewer
{
	// 本地磁盘根目录名称
	const QString LOCAL_DISK_ROOT_FOLDER = "local_disk";

	// 保存全路径的树节点ID
	const int LDT_COLUMN_FULL_PATH = 1;
	const int LDT_COLUMN_NAME = 2;

	// 树节点保存文件全路径
	QString CLocalDiskTree::GetItemFullPath(QTreeWidgetItem* pItem)
	{
		return pItem ? pItem->data(LDT_COLUMN_FULL_PATH, Qt::UserRole).toString() : "null";
	}
	void SetItemFullPath(QTreeWidgetItem* pItem, const QString& strPath)
	{
		if(pItem)
		{
			pItem->setData(LDT_COLUMN_FULL_PATH, Qt::UserRole, QVariant::fromValue(strPath));
		}
	}

	// 树节点保存的文件名（直接使用text时，文件名中含多个空格时存在问题）
	QString GetItemName(QTreeWidgetItem* pItem)
	{
		return pItem ? pItem->data(LDT_COLUMN_NAME, Qt::UserRole).toString() : "null";
	}
	void SetItemName(QTreeWidgetItem* pItem, const QString& strPath)
	{
		if(pItem)
		{
			pItem->setText(0, strPath);
			pItem->setData(LDT_COLUMN_NAME, Qt::UserRole, QVariant::fromValue(strPath));
		}
	}
	
	// 文件全路径
	QString GetFileFullPath(const QFileInfo &fileInfo)
	{
		return fileInfo.filePath();
	}

	// 文件名
	QString GetFileName(const QFileInfo &fileInfo)
	{
		return fileInfo.isRoot() ? fileInfo.filePath() : fileInfo.fileName();
	}

	CLocalDiskTree::CLocalDiskTree( CFreeTreeWidget* parentTreeWidget, const QStringList &strings )
		: m_pParentTreeWidget(parentTreeWidget)
	{
		m_pTreeRootItem = new QTreeWidgetItem(strings);
	}

	CLocalDiskTree::~CLocalDiskTree( void )
	{
		if(m_pParentTreeWidget)
		{
			m_pParentTreeWidget->RemoveTreeItem(m_pTreeRootItem);
		}
	}

	void CLocalDiskTree::Initialize()
	{
		if(m_pParentTreeWidget)
		{
			connect(m_pParentTreeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(SlotItemExpanded(QTreeWidgetItem*)));

			ClearTree();
			AddFolderChildsToTree(LOCAL_DISK_ROOT_FOLDER, m_pTreeRootItem);	
		}
	}

	bool CLocalDiskTree::AddItem( QTreeWidgetItem* pItem, QTreeWidgetItem *pParentItem )
	{
		if(m_pParentTreeWidget)
		{
			return m_pParentTreeWidget->AddTreeItem(pItem, pParentItem);
		}
		return false;
	}

	void CLocalDiskTree::RemoveItem( QTreeWidgetItem* pItem )
	{
		if(m_pParentTreeWidget)
		{
			m_pParentTreeWidget->RemoveTreeItem(pItem);
		}
	}

	// 该函数逻辑目前与业务（数据服务模块）强相关
	void CLocalDiskTree::AddItem( const QFileInfo &fileInfo, QTreeWidgetItem* pParentItem )
	{
		if (fileInfo.isDir())   // 文件夹
		{
			if ( (fileInfo.fileName() == ".") || (fileInfo.fileName() == "..") )
			{
				return;
			}

			QTreeWidgetItem *pChildItem = new QTreeWidgetItem(/*QStringList(GetFileName(fileInfo))*/);
			pChildItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
			pChildItem->setIcon(0, QIcon(":/images/icon/folder.png"));
			pChildItem->setData(0, Qt::UserRole, E_LOCAL_SERVICE_FOLDER);
			
			SetItemName(pChildItem, GetFileName(fileInfo));
			SetItemFullPath(pChildItem, GetFileFullPath(fileInfo));
			AddItem(pChildItem, pParentItem);
		}
		else if (fileInfo.isFile()) // 文件
		{
			// 过滤出img、tif、shp、kml文件
			QString strSuffix = fileInfo.suffix();

			if ( (strSuffix=="img") || (strSuffix=="tif") || (strSuffix=="shp") || (strSuffix=="kml") )
			{
				QTreeWidgetItem *pChildItem = new QTreeWidgetItem(/*QStringList(GetFileName(fileInfo))*/);

				if (strSuffix == "img" || strSuffix == "tif")
				{
					pChildItem->setIcon(0, QIcon(":/images/icon/img.png"));
					pChildItem->setData(0, Qt::UserRole, E_LOCAL_SERVICE_IMAGE_NODE);
				}
				else if (strSuffix == "shp" || strSuffix == "kml")
				{
					pChildItem->setIcon(0, QIcon(":/images/icon/shp.png"));
					pChildItem->setData(0, Qt::UserRole, E_LOCAL_SERVICE_MODEL_NODE);
				}
				
				SetItemName(pChildItem, GetFileName(fileInfo));
				SetItemFullPath(pChildItem, GetFileFullPath(fileInfo));
				AddItem(pChildItem, pParentItem);
			}
		}
	}

	void CLocalDiskTree::AddFolderChildsToTree( QString strFolderPath, QTreeWidgetItem* pParentItem  )
	{
		QFileInfoList fileInfoList;

		if (LOCAL_DISK_ROOT_FOLDER == strFolderPath)
		{
			// 只添加第一级子目录
			fileInfoList = QDir::drives();
		}
		else if(!strFolderPath.isEmpty())
		{
			QDir dir(strFolderPath);
			if (dir.exists())
			{
				// 只添加第一级子目录
				fileInfoList = dir.entryInfoList();
			}
		}

		for(QFileInfoList::iterator iter = fileInfoList.begin(); iter != fileInfoList.end(); iter++)
		{
			AddItem(*iter, pParentItem);
		}
	}

	
	void CLocalDiskTree::SlotItemExpanded( QTreeWidgetItem *pItem )
	{
		if(pItem->childCount() == 0) // 已经展开过，则不再添加
		{
			AddFolderChildsToTree(GetItemFullPath(pItem), pItem);
		}
	}

	// 删除返回true， 否则返回false
	bool CLocalDiskTree::CheckAndRemoveInvalid( QTreeWidgetItem* pItem )
	{
		if( !pItem )
		{
			return false;
		}

		if( pItem != m_pTreeRootItem ) // 根节点不删除
		{
			QDir dir( GetItemFullPath(pItem) );
			if ( !dir.exists() )
			{
				RemoveItem(pItem);
				return true;
			}
		}

		// 如果展开，则检查子节点
		if( pItem->isExpanded() )
		{
			for(int n = 0; n < pItem->childCount(); n++)
			{
				if ( CheckAndRemoveInvalid(pItem->child(n)) )
				{
					--n; // 该项被删除，索引回退
				}
			}
		}
		else
		{
			// 删除已经折叠的子项
			m_pParentTreeWidget->RemoveTreeItemChilds(pItem);
		}
		
		return false;
	}

	// 查找与指定名匹配的节点，存在返回true，否则返回false
	bool FindChildItemByName(QTreeWidgetItem* pItemParent, const QString& name)
	{
		for(int n = 0; n < pItemParent->childCount(); n++)
		{
			if( name == GetItemName(pItemParent->child(n)) )
			{
				return true;
			}
		}
		return false;
	}
	
	void CLocalDiskTree::CheckAndInsertNew( QTreeWidgetItem* pItem )
	{
		// 前置条件： 目前树上保留的项都是有效的
		// 只检查展开项，因为折叠项已经被上一步操作 CheckAndRemoveInvalid 删除
		if( pItem && pItem->isExpanded() )
		{
			QFileInfoList fileInfoList;

			if( pItem == m_pTreeRootItem )
			{
				fileInfoList = QDir::drives();
			}
			else
			{
				QDir dir( GetItemFullPath(pItem) );
				if ( dir.exists() )
				{
					fileInfoList = dir.entryInfoList();
				}
			}

			// 在树中查找每一个子项，不存在则添加
			for(QFileInfoList::iterator iter = fileInfoList.begin(); iter != fileInfoList.end(); iter++)
			{
				QString fileName = GetFileName(*iter);

				if(!FindChildItemByName(pItem, fileName))
				{
					AddItem(*iter, pItem);
				}
			}

			// 递归检查子项
			for(int n = 0; n < pItem->childCount(); n++)
			{
				CheckAndInsertNew(pItem->child(n));
			}
		}
	}

	void CLocalDiskTree::RescanDisk()
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutexScanDisk);
		
		/*
			本来重新扫描磁盘最简单的逻辑是： 清除之前的节点，并重新加载
			但是，为了支持“保留已经展开的树节点状态”，所以需要进行以下处理。
		*/

		// 首先遍历所有已经展开的目录，删除不存在项的和折叠项的孩子节点
		CheckAndRemoveInvalid(m_pTreeRootItem);

		// 添加新增项
		CheckAndInsertNew(m_pTreeRootItem);
	}

	void CLocalDiskTree::ClearTree()
	{
		if(m_pParentTreeWidget && m_pTreeRootItem)
		{
			m_pParentTreeWidget->RemoveTreeItemChilds(m_pTreeRootItem);
		}
	}

}