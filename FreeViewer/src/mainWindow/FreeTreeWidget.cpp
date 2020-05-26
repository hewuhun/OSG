#include <mainWindow/FreeTreeWidget.h>

#include <QHoverEvent>
#include <QHeaderView>

namespace FreeViewer
{
	CFreeTreeWidget::CFreeTreeWidget(QWidget *parent, bool isService)
		: QTreeWidget(parent)
		, m_bIsService(isService)
	{
		InitWidget();
	}

	CFreeTreeWidget::~CFreeTreeWidget(void)
	{

	}

	void CFreeTreeWidget::InitWidget()
	{
		// 设置标头
		this->header()->hide();
#ifdef __QT4_USE_UNICODE
		this->header()->setResizeMode(QHeaderView::ResizeToContents);
#else
		this->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
		this->header()->setStretchLastSection(false);

		// 开启滚动条
		this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		this->setAutoScroll(false);

		// 去除光标
		//this->setFocusPolicy(Qt::NoFocus);

		// 禁止编辑
		this->setEditTriggers(QAbstractItemView::NoEditTriggers);

		//关联树节点变化的信号和槽函数
		connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(SlotItemChecked(QTreeWidgetItem*, int)));
	}

	bool CFreeTreeWidget::AddTopTreeItem( QTreeWidgetItem* pItem )
	{
		if (NULL == pItem)
		{
			return false;
		}

		addTopLevelItem(pItem); 

		return true;
	}

	bool CFreeTreeWidget::AddTreeItem( QTreeWidgetItem* pItem, QTreeWidgetItem *pParent )
	{
		if (NULL == pParent)
		{
			return false;
		}

		pParent->addChild(pItem);
		SlotItemChecked(pItem, 0);

		return true;
	}

	void CFreeTreeWidget::RemoveTreeItem( QTreeWidgetItem* pItem )
	{
		RemoveItemPrivate(pItem);
	}

	void CFreeTreeWidget::RemoveTreeItemChilds( QTreeWidgetItem* pItem )
	{
		RemoveItemChildsPrivate(pItem);
	}

	void CFreeTreeWidget::RemoveItemPrivate( QTreeWidgetItem* pItem )
	{
		if(NULL == pItem) 
		{
			return;
		}

		while(pItem->childCount() > 0)
		{
			QTreeWidgetItem* pDeleteItem = pItem->child(0);
			RemoveItemPrivate(pDeleteItem);
		}

		delete pItem;
		pItem = NULL;
	}

	void CFreeTreeWidget::RemoveItemChildsPrivate( QTreeWidgetItem* pItem )
	{
		if(NULL == pItem)
		{
			return;
		}

		/// delete children
		while(pItem->childCount() > 0)
		{
			QTreeWidgetItem* pDeleteItem = pItem->child(0);
			RemoveItemPrivate(pDeleteItem);
		}
		//子节点删除后 点击状态为unchecked
		pItem->setCheckState(0, Qt::Unchecked);
	}

	void CFreeTreeWidget::contextMenuEvent(QContextMenuEvent *event)
	{
		QTreeWidgetItem* item = itemAt(event->pos());
		if (NULL != item)
		{
			emit SignalContextMenuItem(item, event->globalPos());
		}
	}

	void CFreeTreeWidget::SlotItemChecked(QTreeWidgetItem* pItem, int nIndex)
	{
		if (m_bIsService)
		{
		   pItem->setData(0, Qt::CheckStateRole, QVariant());
		   emit SignalItemsVisibleChange(pItem, true);
		}
		else
		{
			bool bShow = pItem->checkState(0);
			emit SignalItemsVisibleChange(pItem, bShow);

			disconnect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(SlotItemChecked(QTreeWidgetItem*, int)));
			UpdateChildItemState(pItem);
			UpdateParentItemState(pItem);
			connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(SlotItemChecked(QTreeWidgetItem*, int)));
		}
		
	
	}

	void CFreeTreeWidget::UpdateChildItemState(QTreeWidgetItem* pItem)
	{
		if (pItem && pItem->childCount())
		{
			for (int i = 0; i < pItem->childCount(); i++)
			{
				QTreeWidgetItem* pChildItem = pItem->child(i);
				if (NULL == pChildItem)
				{
					continue;
				}

				if (pChildItem->checkState(0) != pItem->checkState(0))
				{
					pChildItem->setCheckState(0, pItem->checkState(0));

					bool bShow = pItem->checkState(0);
					emit SignalItemsVisibleChange(pChildItem, bShow);
				}

				UpdateChildItemState(pChildItem);
			}
		}
	}

	void CFreeTreeWidget::UpdateParentItemState(QTreeWidgetItem* pItem)
	{
		if (pItem->parent())
		{
			// 屏蔽根节点复选框
			//if (!(pItem->parent())->parent())
			//{
			//	return;
			//}

			unsigned int nCheckeds = 0;
			unsigned int nUnCheckeds = 0;

			unsigned int nCounts = pItem->parent()->childCount();
			for (unsigned int i = 0; i < nCounts; i++)
			{
				if (pItem->parent()->child(i)->checkState(0) == Qt::Checked)
				{
					nCheckeds++;
				}
				else if(pItem->parent()->child(i)->checkState(0) == Qt::Unchecked)
				{
					nUnCheckeds++;
				}
			}

			if (nCheckeds == nCounts)
			{
				if (pItem->parent()->checkState(0) != Qt::Checked)
				{
					pItem->parent()->setCheckState(0, Qt::Checked);

					//zs
					emit SignalItemsVisibleChange(pItem->parent(), true);
				}
			}
			else if(nUnCheckeds == nCounts)
			{
				if (pItem->parent()->checkState(0) != Qt::Unchecked)
				{
					pItem->parent()->setCheckState(0, Qt::Unchecked);

					//zs
					emit SignalItemsVisibleChange(pItem->parent(), false);
				}
			}
			else
			{
				if (pItem->parent()->checkState(0) != Qt::PartiallyChecked)
				{
					pItem->parent()->setCheckState(0, Qt::PartiallyChecked);
					//zs
					emit SignalItemsVisibleChange(pItem->parent(), true);
				}
			}

			UpdateParentItemState(pItem->parent());
		}
	}

	void CFreeTreeWidget::keyPressEvent( QKeyEvent *event )
	{
		emit SignalKeyPressed(event);
	}

	void CFreeTreeWidget::dropEvent( QDropEvent *event )
	{
		// 有效性校验，解决拖动项消失问题（g00066 2018.09.19）
		QModelIndex droppedIndex = indexAt(event->pos());
		if( !droppedIndex.isValid() ) 
		{
 			event->setDropAction(Qt::IgnoreAction);
 			event->accept();
			return;
		}

		emit SignalDropEvent(event);
	}

	void CFreeTreeWidget::dragMoveEvent( QDragMoveEvent *event )
	{
		emit SignalDragMoveEvent(event);
	}

	bool CFreeTreeWidget::InsertFrontTreeItem( QTreeWidgetItem* pItem, QTreeWidgetItem *pParent )
	{
		if (NULL == pParent)
		{
			return false;
		}

		pParent->insertChild(0, pItem);
		SlotItemChecked(pItem, 0);

		return true;
	}

}
