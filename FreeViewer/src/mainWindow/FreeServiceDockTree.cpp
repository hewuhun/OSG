#include <mainWindow/FreeServiceDockTree.h>
#include <service/ServicePluginInterface.h>

namespace FreeViewer
{
	CFreeServiceDockTree::CFreeServiceDockTree( QString titleText, QWidget *parent /*= 0*/ ) 
		: CFreeDockWidget(titleText, parent)
		, m_pServiceTreeWidget(NULL)
		, m_pServiceButton(NULL)
	{
		InitWidget();
	}

	CFreeServiceDockTree::~CFreeServiceDockTree()
	{

	}

	void CFreeServiceDockTree::InitWidget()
	{
		m_pServiceTreeWidget = new CFreeTreeWidget(this, true);

		// 开启拖拽
		m_pServiceTreeWidget->setDragEnabled(true);
		// 关闭放置
		m_pServiceTreeWidget->setAcceptDrops(false);
		// 设置模式
		m_pServiceTreeWidget->setDragDropMode(QAbstractItemView::DragDrop);
		AddSubWidget(m_pServiceTreeWidget);

		m_pServiceButton = new CFreeServiceButton(this);
		AddSubWidget(m_pServiceButton);

		// 关联树节点信号和槽
		connect(m_pServiceTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(SlotItemClicked(QTreeWidgetItem*, int)));
		connect(m_pServiceTreeWidget, SIGNAL(itemPressed(QTreeWidgetItem*, int)), this, SLOT(SlotItemClicked(QTreeWidgetItem*, int)));
		connect(m_pServiceTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(SlotItemDoubleClicked(QTreeWidgetItem*, int)));
		connect(m_pServiceTreeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(SlotItemExpanded(QTreeWidgetItem*)));
		connect(m_pServiceTreeWidget, SIGNAL(SignalContextMenuItem(QTreeWidgetItem*, QPoint)), this, SLOT(SlotContextMenuItem(QTreeWidgetItem*, QPoint)));
		connect(m_pServiceTreeWidget, SIGNAL(SignalKeyPressed(QKeyEvent*)), this, SLOT(SlotKeyPressed(QKeyEvent*)));
		connect(m_pServiceTreeWidget, SIGNAL(SignalItemsVisibleChange(QTreeWidgetItem*, bool)), this, SLOT(SlotItemVisibleChange(QTreeWidgetItem*, bool)));
		connect(m_pServiceTreeWidget, SIGNAL(SignalDragMoveEvent(QDragMoveEvent*)), this, SLOT(SlotDragMoveEvent(QDragMoveEvent*)));

		// 关联停靠窗按钮信号和槽
		connect(m_pServiceButton, SIGNAL(SignalUpdateBtnClicked(EServiceButtonType)), this, SLOT(SlotBtnClicked(EServiceButtonType)));
		connect(m_pServiceButton, SIGNAL(SignalExpandBtnClicked(EServiceButtonType)), this, SLOT(SlotBtnClicked(EServiceButtonType)));
		connect(m_pServiceButton, SIGNAL(SignalCollapseBtnClicked(EServiceButtonType)), this, SLOT(SlotBtnClicked(EServiceButtonType)));
		connect(m_pServiceButton, SIGNAL(SignalDeleteBtnClicked(EServiceButtonType)), this, SLOT(SlotBtnClicked(EServiceButtonType)));
	}

	CFreeTreeWidget* CFreeServiceDockTree::GetTreeWidget()
	{
		return m_pServiceTreeWidget;
	}

	void CFreeServiceDockTree::PushBackSystem( EServiceItemType type, CUIObserver* pSystem )
	{
		m_mapTypeService.insert(std::make_pair(type, pSystem));
	}

	void CFreeServiceDockTree::SlotContextMenuItem( QTreeWidgetItem* pItem, QPoint point )
	{
		if (NULL == pItem)
		{
			return;
		}

		TypeServiceMap::iterator it = m_mapTypeService.find(E_SERVICE);
		if (it != m_mapTypeService.end())
		{
			CServicePluginInterface* pServicePluginInterface = dynamic_cast<CServicePluginInterface*>(it->second);
			if (pServicePluginInterface)
			{
				pServicePluginInterface->ContextMenuItem(pItem, point);
			}
		}
	}

	void CFreeServiceDockTree::SlotItemClicked( QTreeWidgetItem *pItem, int column )
	{
		DragItem();

		TypeServiceMap::iterator it = m_mapTypeService.find(E_SERVICE);
		if (it != m_mapTypeService.end())
		{
			CServicePluginInterface* pServicePluginInterface = dynamic_cast<CServicePluginInterface*>(it->second);
			if (pServicePluginInterface)
			{
				pServicePluginInterface->ItemClicked(pItem);
			}
		}
	}

	void CFreeServiceDockTree::SlotItemDoubleClicked( QTreeWidgetItem *pItem, int column )
	{
		if (NULL == pItem)
		{
			return;
		}

		TypeServiceMap::iterator it = m_mapTypeService.find(E_SERVICE);
		if (it != m_mapTypeService.end())
		{
			CServicePluginInterface* pServicePluginInterface = dynamic_cast<CServicePluginInterface*>(it->second);
			if (pServicePluginInterface)
			{
				pServicePluginInterface->ItemDoubleClicked(pItem);
			}
		}
	}

	void CFreeServiceDockTree::SlotBtnClicked( EServiceButtonType type )
	{
		TypeServiceMap::iterator it = m_mapTypeService.find(E_SERVICE);
		if (it != m_mapTypeService.end())
		{
			CServicePluginInterface* pServicePluginInterface = dynamic_cast<CServicePluginInterface*>(it->second);
			if (pServicePluginInterface)
			{
				pServicePluginInterface->BtnClicked(type);
			}
		}
	}

	void CFreeServiceDockTree::SlotKeyPressed( QKeyEvent *event )
	{
		if (event->key() == Qt::Key_R)
		{
			TypeServiceMap::iterator it = m_mapTypeService.find(E_SERVICE);
			if (it != m_mapTypeService.end())
			{
				CServicePluginInterface* pServicePluginInterface = dynamic_cast<CServicePluginInterface*>(it->second);
				if (pServicePluginInterface)
				{
					pServicePluginInterface->RKeyPressed();
				}
			}
		}
		else if (event->key() == Qt::Key_F2)
		{
			TypeServiceMap::iterator it = m_mapTypeService.find(E_SERVICE);
			if (it != m_mapTypeService.end())
			{
				CServicePluginInterface* pServicePluginInterface = dynamic_cast<CServicePluginInterface*>(it->second);
				if (pServicePluginInterface)
				{
					pServicePluginInterface->F2KeyPressed();
				}
			}
		}
		else if (event->key() == Qt::Key_Delete)
		{
			TypeServiceMap::iterator it = m_mapTypeService.find(E_SERVICE);
			if (it != m_mapTypeService.end())
			{
				CServicePluginInterface* pServicePluginInterface = dynamic_cast<CServicePluginInterface*>(it->second);
				if (pServicePluginInterface)
				{
					pServicePluginInterface->DeleteKeyPressed();
				}
			}
		}
	}

	void CFreeServiceDockTree::SlotItemVisibleChange( QTreeWidgetItem* pItem, bool bState )
	{
		if (NULL == pItem)
		{
			return;
		}

		TypeServiceMap::iterator it = m_mapTypeService.find(E_SERVICE);
		if (it != m_mapTypeService.end())
		{
			CServicePluginInterface* pServicePluginInterface = dynamic_cast<CServicePluginInterface*>(it->second);
			if (pServicePluginInterface)
			{
				pServicePluginInterface->EditRenameItem(pItem, bState);
			}
		}
	}

	void CFreeServiceDockTree::SlotDragMoveEvent( QDragMoveEvent *event )
	{
		event->ignore();
		return;
	}

	void CFreeServiceDockTree::DragItem()
	{
		QTreeWidgetItem *pCurrentItem = m_pServiceTreeWidget->currentItem();
		if (pCurrentItem)
		{
			EServiceItemType type = (EServiceItemType)pCurrentItem->data(0, Qt::UserRole).toInt();
			switch (type)
			{
			case E_LOCAL_SERVICE_ROOT:
			case E_LOCAL_SERVICE_FOLDER:
			case E_OGC_SERVICE_ROOT:
			case E_WMS_SERVICE_ROOT:
			case E_WMTS_SERVICE_ROOT:
			case E_WFS_SERVICE_ROOT:
			case E_WCS_SERVICE_ROOT:
			case E_OGC_SERVICE_FOLDER:
			case E_TILE_SERVICE_ROOT:
			case E_TMS_SERVICE_ROOT:
			case E_XYZ_SERVICE_ROOT:
			case E_DATA_SERVICE_ROOT:
				{
					emit SignalDragItem(false);
					return;
				}
			case E_LOCAL_SERVICE_IMAGE_NODE:
			case E_LOCAL_SERVICE_MODEL_NODE:
			case E_WMS_SERVICE_NODE:
			case E_WMTS_SERVICE_NODE:
			case E_WFS_SERVICE_NODE:
			case E_WCS_SERVICE_NODE:
			case E_TMS_SERVICE_NODE:
			case E_XYZ_SERVICE_NODE:
			case E_DATA_SERVICE_NODE:
				{
					emit SignalDragItem(true);
					return;
				}
			}
		}
	}

	void CFreeServiceDockTree::SlotItemExpanded( QTreeWidgetItem *pItem )
	{
		if (NULL == pItem)
		{
			return;
		}

		TypeServiceMap::iterator it = m_mapTypeService.find(E_SERVICE);
		if (it != m_mapTypeService.end())
		{
			CServicePluginInterface* pServicePluginInterface = dynamic_cast<CServicePluginInterface*>(it->second);
			if (pServicePluginInterface)
			{
				pServicePluginInterface->ItemDoubleClicked(pItem);
				m_pServiceTreeWidget->repaint();
			}
		}
	}

}
