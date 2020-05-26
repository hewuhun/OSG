#include <viewPoint/ViewPointTree.h>

#include <viewPoint/ViewPointDialog.h>
#include <viewPoint/ViewPointSerializer.h>

#include <mainWindow/FreeMainWindow.h>

#include <FeUtils/PathRegistry.h>

namespace FreeViewer
{
	CViewPointTree::CViewPointTree(CFreeMainWindow* pMainWindow, std::string strDataPath)
		:CUIObserver(pMainWindow)
		,m_p3DTreeWidget(NULL)
		,m_pViewPointItemRoot(NULL)
		,m_pAddViewPointAction(NULL)
		,m_pDeleteViewPointAction(NULL)
		,m_pClearFolderAction(NULL)
		,m_pActiveItem(NULL)
		,m_strDataPath(strDataPath)
	{
		InitWidget();
	}

	CViewPointTree::~CViewPointTree(void)
	{
		SaveData();
	}

	void CViewPointTree::InitWidget()
	{
		if (NULL == m_pMainWindow)
		{
			return ;
		}

		m_p3DTreeWidget = m_pMainWindow->Get3DTreeWidget();
		if (NULL == m_p3DTreeWidget)
		{
			return ;
		}
		m_p3DTreeWidget->PushBackSystem(E_VIEWPOINT_LAYER, this);

		//标题栏右键菜单，清空图层文件夹
		m_pClearFolderAction = new QAction(tr("Clear ViewPoint Folder"), this);
		m_pClearFolderAction->setToolTip(tr("Clear ViewPoint Folder"));
		m_pClearFolderAction->setDisabled(false);
		connect(m_pClearFolderAction, SIGNAL(triggered()), this, SLOT(SlotClearLayerFolder()));

		//标题栏右键菜单，添加子节点
		m_pAddViewPointAction = new QAction(tr("Add ViewPoint"), this);
		m_pAddViewPointAction->setToolTip(tr("Add ViewPoint"));
		m_pAddViewPointAction->setDisabled(false);
		connect(m_pAddViewPointAction, SIGNAL(triggered()), this, SLOT(SlotAddViewPoint()));

		//树节点右键菜单，删除子节点
		m_pDeleteViewPointAction = new QAction(tr("Delete ViewPoint"), this);
		m_pDeleteViewPointAction->setToolTip(tr("Delete ViewPoint"));
		m_pDeleteViewPointAction->setDisabled(false);
		connect(m_pDeleteViewPointAction, SIGNAL(triggered()), this, SLOT(SlotDeleteViewPoint()));

		BuildContext();
	}

	void CViewPointTree::BuildContext()
	{
		m_pViewPointItemRoot = new QTreeWidgetItem(m_p3DTreeWidget->GetTreeWidget(), QStringList(tr("ViewPoint Layer")), 0);
		m_pViewPointItemRoot->setData(0, Qt::UserRole, E_VIEWPOINT_LAYER_ROOT);
		m_pViewPointItemRoot->setIcon(0, QIcon(":images/icon/viewpoint_set.png"));
		m_p3DTreeWidget->GetTreeWidget()->AddTopTreeItem(m_pViewPointItemRoot);

		LoadData();

		m_pViewPointItemRoot->setExpanded(true);
	}

	void CViewPointTree::LoadData()
	{
		CViewPointReaderWriter rw;
		FreeViewPoints viewPoints = rw.Load(m_strDataPath);

		std::list<CFreeViewPoint>::iterator it = viewPoints.begin();
		while (it != viewPoints.end())
		{
			CFreeViewPoint point = *it;
			QTreeWidgetItem* pItem = new QTreeWidgetItem(m_pViewPointItemRoot, QStringList(point.GetName().c_str()));
			pItem->setIcon(0, QIcon(":/images/icon/to_viewpoint.png"));
			pItem->setData(0, Qt::UserRole, QVariant(E_VIEWPOINT_LAYER_NODE));
			pItem->setData(1, Qt::UserRole, QVariant::fromValue(point));
			m_p3DTreeWidget->GetTreeWidget()->AddTreeItem(pItem, m_pViewPointItemRoot);

			it++;
		}
	}

	void CViewPointTree::SaveData()
	{
		CViewPointReaderWriter rw;

		int nCount = m_pViewPointItemRoot->childCount();
		for (int i = 0; i < nCount; i++)
		{
			QTreeWidgetItem* pItem = m_pViewPointItemRoot->child(i);
			if (pItem)
			{
				CFreeViewPoint point = pItem->data(1, Qt::UserRole).value<CFreeViewPoint>();
				rw.AddViewPoint(point);
			}
		}

		rw.Save(m_strDataPath);
	}

	void CViewPointTree::ClearItem( QTreeWidgetItem* pItem )
	{
		if (NULL == pItem)
		{
			return;
		}

		int nCount = pItem->childCount();
		for (int i = 0; i < nCount; i++)
		{
			//删除一个孩子后，下次进来孩子数量就少一个
			QTreeWidgetItem* pChild = pItem->child(0);
			if (pChild)
			{
				EItemType type = (EItemType)(pChild->data(0, Qt::UserRole).toInt());
				switch(type)
				{
				case E_VIEWPOINT_LAYER_FOLDER:
					{
						ClearItem(pChild);
						m_p3DTreeWidget->GetTreeWidget()->RemoveTreeItem(pChild);
					}
					break;

				case E_VIEWPOINT_LAYER_NODE:
					{
						m_p3DTreeWidget->GetTreeWidget()->RemoveTreeItem(pChild);
					}
					break;

				default:
					break;
				}
			}
		}
	}

	void CViewPointTree::DeleteItem(QTreeWidgetItem* pItem)
	{
		if (NULL == pItem)
		{
			return;
		}

		//首先清空该节点
		ClearItem(pItem);

		//然后移除自身
		m_p3DTreeWidget->GetTreeWidget()->RemoveTreeItem(pItem);
	}

	void CViewPointTree::SlotAddViewPoint()
	{
		CViewPointDialog *m_pViewPointDialog = new CViewPointDialog(m_opSystemService.get(), m_pMainWindow);
		m_pViewPointDialog->SetTitleText(tr("View Point"));
		connect(m_pViewPointDialog, SIGNAL(SignalAddViewPoint(FeUtil::CFreeViewPoint)),
			this, SLOT(SlotAddViewPoint(FeUtil::CFreeViewPoint)));

		m_pViewPointDialog->ShowDialogNormal();
	}

	void CViewPointTree::SlotAddViewPoint(FeUtil::CFreeViewPoint point)
	{
		QTreeWidgetItem* pItem = new QTreeWidgetItem(m_pViewPointItemRoot, QStringList(QString(point.GetName().c_str())));
		pItem->setIcon(0, QIcon(":/images/icon/to_viewpoint.png"));
		pItem->setData(0, Qt::UserRole, QVariant(E_VIEWPOINT_LAYER_NODE));
		pItem->setData(1, Qt::UserRole, QVariant::fromValue(point));

		m_p3DTreeWidget->GetTreeWidget()->AddTreeItem(pItem, m_pViewPointItemRoot);
		
		m_pViewPointItemRoot->setExpanded(true);
	}

	void CViewPointTree::SlotDeleteViewPoint()
	{
		if (NULL == m_pActiveItem)
		{
			return;
		}

		DeleteItem(m_pActiveItem);
		//SaveData();
	}

	void CViewPointTree::SlotClearLayerFolder()
	{
		if (NULL == m_pActiveItem)
		{
			return;
		}

		ClearItem(m_pActiveItem);
		//SaveData();
	}

	void CViewPointTree::SlotContextMenuItem(QTreeWidgetItem* pItem, QPoint pos)
	{
		if (NULL == pItem)
		{
			return ;
		}
		m_pActiveItem = pItem;

		EItemType type = (EItemType)pItem->data(0, Qt::UserRole).toInt();
		//如果Item的类型是文件夹或者图层
		if (E_VIEWPOINT_LAYER_NODE == type)
		{
			QMenu* pPopMenu = new QMenu(m_p3DTreeWidget->GetTreeWidget());
			pPopMenu->addAction(m_pDeleteViewPointAction);

			pPopMenu->exec(pos);
		}
		//如果Item的类型是图层根节点，则创建添加视点和清除文件夹
		else if (E_VIEWPOINT_LAYER_ROOT == type)
		{
			QMenu* pPopMenu = new QMenu(m_p3DTreeWidget->GetTreeWidget());
			pPopMenu->addAction(m_pAddViewPointAction);
			pPopMenu->addAction(m_pClearFolderAction);

			pPopMenu->exec(pos);
		}
	}

	void CViewPointTree::SlotItemDBClicked(QTreeWidgetItem* pItem)
	{
		if (NULL == pItem)
		{
			return ;
		}
		m_pActiveItem = pItem;

		EItemType type = (EItemType)pItem->data(0, Qt::UserRole).toInt();
		if (E_VIEWPOINT_LAYER_NODE == type)
		{
			FeUtil::CFreeViewPoint point = pItem->data(1, Qt::UserRole).value<FeUtil::CFreeViewPoint>();
			m_opSystemService->Locate(point);
		}
	}

}
