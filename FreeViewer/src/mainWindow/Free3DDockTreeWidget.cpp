#include <mainWindow/Free3DDockTreeWidget.h>

#include <layer/SystemLayerTree.h>
#include <layer/CustomLayerTree.h>
#include <mark/MarkPluginInterface.h>
#include <viewPoint/ViewPointTree.h>
#include <plot/PlotPluginInterface.h>

//单个树节点高度
const int ITEM_HEIGHT = 25;
//树型控件框向下偏移的像素大小
const int TREE_WIDGET_OFFSET = 15; 

namespace FreeViewer
{
	CFree3DDockTreeWidget::CFree3DDockTreeWidget( QString titleText, QWidget *parent /*= 0*/ ) 
		: CFreeDockWidget(titleText, parent)
		,m_p3DSceneTreeWidget(NULL)
	{
		InitWidget();
	}

	CFree3DDockTreeWidget::~CFree3DDockTreeWidget()
	{

	}

	void CFree3DDockTreeWidget::InitWidget()
	{
		m_p3DSceneTreeWidget = new CFreeTreeWidget(this);

		//关联树节点展开信号和槽
		connect(m_p3DSceneTreeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(SlotUpdateScroll()));
		//关联树节点合并信号和槽
		connect(m_p3DSceneTreeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(SlotUpdateScroll()));
		//关联树节点状态更新信号和槽
		connect(m_p3DSceneTreeWidget, SIGNAL(SignalTreeStateChanged()), this, SLOT(SlotUpdateScroll()));

		//连接右键菜单信号和槽
		connect(m_p3DSceneTreeWidget, SIGNAL(SignalContextMenuItem(QTreeWidgetItem*, QPoint)),
			this, SLOT(SlotContextMenuItem(QTreeWidgetItem*, QPoint)));
		//连接Item选中和未选中的信号和槽
		connect(m_p3DSceneTreeWidget, SIGNAL(SignalItemsVisibleChange(QTreeWidgetItem*, bool)),
			this, SLOT(SlotItemVisibleChange(QTreeWidgetItem*, bool)));
		//连接Item双击选中事件
		connect(m_p3DSceneTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), 
			this, SLOT(SlotItemDoubleClicked(QTreeWidgetItem*)));

		SlotUpdateScroll();
	}

	CFreeTreeWidget* CFree3DDockTreeWidget::GetTreeWidget()
	{
		return m_p3DSceneTreeWidget;
	}

	void CFree3DDockTreeWidget::PushBackSystem( EItemType type, CUIObserver* pSystem )
	{
		m_mapTypeSystem.insert(std::make_pair(type, pSystem));
	}

	void CFree3DDockTreeWidget::SlotUpdateScroll()
	{
		//int nHeight = m_p3DSceneTreeWidget->GetItemsNums() * ITEM_HEIGHT + TREE_WIDGET_OFFSET;
		//int nWidth = 266;
		//m_p3DSceneTreeWidget->setFixedSize(nWidth, nHeight);

		//ui.scrollArea->setWidget(m_p3DSceneTreeWidget);
		//ui.scrollArea->setWidgetResizable(true);
		ui.verticalLayout_Widget->addWidget(m_p3DSceneTreeWidget);
	}

	void CFree3DDockTreeWidget::SlotContextMenuItem( QTreeWidgetItem* pItem, QPoint point )
	{
		if (NULL == pItem)
		{
			return;
		}

		EItemType type = (EItemType)(pItem->data(0, Qt::UserRole).toInt());
		switch(type)
		{
		case E_MARK_FOLDER:
		case E_MARK_LAYER_ROOT:
		case E_MARK_NODE:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_MARK_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CMarkPluginInterface* pMarkPlugin = dynamic_cast<CMarkPluginInterface*>(it->second);
					if (pMarkPlugin)
					{
						pMarkPlugin->SlotContextMenuItem(pItem, point);
					}
				}
			}
			break;

		case E_PLOT_FOLDER:
		case E_PLOT_LAYER_ROOT:
		case E_PLOT_NODE:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_PLOT_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CPlotPluginInterface* pPlotPlugin = dynamic_cast<CPlotPluginInterface*>(it->second);
					if (pPlotPlugin)
					{
						pPlotPlugin->SlotContextMenuItem(pItem, point);
					}
				}
			}
			break;

		case E_VIEWPOINT_LAYER_NODE:
		case E_VIEWPOINT_LAYER_FOLDER:
		case E_VIEWPOINT_LAYER_ROOT:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_VIEWPOINT_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CViewPointTree* pViewPointLayer = dynamic_cast<CViewPointTree*>(it->second);
					if (pViewPointLayer)
					{
						pViewPointLayer->SlotContextMenuItem(pItem, point);
					}
				}
			}
			break;

		case E_CUSTOM_LAYER_NODE:
		case E_CUSTOM_LAYER_FOLDER:
		case E_CUSTOM_LAYER_ROOT:
		case E_CUSTOM_IMAGE_LAYER_ROOT:
		case E_CUSTOM_ELEVATION_LAYER_ROOT:
		case E_CUSTOM_VECTOR_LAYER_ROOT:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_CUSTOM_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CCustomLayerTree* pCustomLayer = dynamic_cast<CCustomLayerTree*>(it->second);
					if (pCustomLayer)
					{
						pCustomLayer->SlotContextMenuItem(pItem, point);
					}
				}
			}
			break;

		case E_SYSTEM_LAYER_NODE:
		case E_SYSTEM_LAYER_ROOT:
		case E_SYSTEM_LAYER_IMAGE_FOLDER:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_SYSTEM_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CSystemLayerTree* pSystemLayer = dynamic_cast<CSystemLayerTree*>(it->second);
					if (pSystemLayer)
					{
						pSystemLayer->SlotContextMenuItem(pItem, point);
					}
				}
			}
			break;

		default:
			break;
		}
	}

	void CFree3DDockTreeWidget::SlotItemVisibleChange( QTreeWidgetItem* pItem, bool bState )
	{
		if (NULL == pItem)
		{
			return;
		}

		EItemType type = (EItemType)(pItem->data(0, Qt::UserRole).toInt());
		switch(type)
		{
		case E_MARK_FOLDER:
		case E_MARK_LAYER_ROOT:
		case E_MARK_NODE:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_MARK_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CMarkPluginInterface* pMarkPlugin = dynamic_cast<CMarkPluginInterface*>(it->second);
					if (pMarkPlugin)
					{
						pMarkPlugin->SlotItemsVisibleChange(pItem, bState);
					}
				}
			}
			break;

		case E_PLOT_FOLDER:
		case E_PLOT_LAYER_ROOT:
		case E_PLOT_NODE:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_PLOT_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CPlotPluginInterface* pPlotPlugin = dynamic_cast<CPlotPluginInterface*>(it->second);
					if (pPlotPlugin)
					{
						pPlotPlugin->SlotItemsVisibleChange(pItem, bState);
					}
				}
			}
			break;

		case E_CUSTOM_LAYER_NODE:
		case E_CUSTOM_LAYER_FOLDER:
		case E_CUSTOM_LAYER_ROOT:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_CUSTOM_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CCustomLayerTree* pCustomLayer = dynamic_cast<CCustomLayerTree*>(it->second);
					if (pCustomLayer)
					{
						pCustomLayer->SlotItemsVisibleChange(pItem, bState);
					}
				}
			}
			break;

		case E_SYSTEM_LAYER_NODE:
		case E_SYSTEM_LAYER_ROOT:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_SYSTEM_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CSystemLayerTree* pSystemLayer = dynamic_cast<CSystemLayerTree*>(it->second);
					if (pSystemLayer)
					{
						pSystemLayer->SlotItemsVisibleChange(pItem, bState);
					}
				}
			}
			break;

		default:
			break;
		}
	}

	void CFree3DDockTreeWidget::SlotItemDoubleClicked( QTreeWidgetItem* pItem )
	{
		if (NULL == pItem)
		{
			return;
		}

		EItemType type = (EItemType)(pItem->data(0, Qt::UserRole).toInt());
		switch(type)
		{
		case E_MARK_FOLDER:
		case E_MARK_LAYER_ROOT:
		case E_MARK_NODE:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_MARK_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CMarkPluginInterface* pMarkPlugin = dynamic_cast<CMarkPluginInterface*>(it->second);
					if (pMarkPlugin)
					{
						pMarkPlugin->SlotItemDBClicked(pItem);
					}
				}
			}
			break;

		case E_PLOT_FOLDER:
		case E_PLOT_LAYER_ROOT:
		case E_PLOT_NODE:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_PLOT_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CPlotPluginInterface* pPlotPlugin = dynamic_cast<CPlotPluginInterface*>(it->second);
					if (pPlotPlugin)
					{
						pPlotPlugin->SlotItemDBClicked(pItem);
					}
				}
			}
			break;

		case E_VIEWPOINT_LAYER_FOLDER:
		case E_VIEWPOINT_LAYER_ROOT:
		case E_VIEWPOINT_LAYER_NODE:
			{
				TypeSystemMap::iterator it = m_mapTypeSystem.find(E_VIEWPOINT_LAYER);
				if (it != m_mapTypeSystem.end())
				{
					CViewPointTree* pViewPointLayer = dynamic_cast<CViewPointTree*>(it->second);
					if (pViewPointLayer)
					{
						pViewPointLayer->SlotItemDBClicked(pItem);
					}
				}
			}
			break;

		case E_CUSTOM_LAYER_NODE:
		case E_CUSTOM_LAYER_FOLDER:
		case E_CUSTOM_LAYER_ROOT:

			break;

		case E_SYSTEM_LAYER_NODE:
		case E_SYSTEM_LAYER_ROOT:

			break;

		default:
			break;
		}
	}

}
