#include <mainWindow/FreeSceneWidget.h>

#include <mainWindow/FreeUtil.h>

#include <QPainter>
#include <QPen>

namespace FreeViewer
{
	CFreeSceneWidget::CFreeSceneWidget( QWidget* parent )
		:QSplitter(Qt::Orientation::Horizontal,parent)
	{
		InitWidget();

		InitContext();
	}

	CFreeSceneWidget::~CFreeSceneWidget()
	{

	}

	void CFreeSceneWidget::InitWidget()
	{
		setContentsMargins(2, 2, 2, 2);
		setChildrenCollapsible(false);

		SetStyleSheet(this, ":/css/scenewidget.qss");
	}

	void CFreeSceneWidget::InitContext()
	{

	}

	void CFreeSceneWidget::AddSubWidget( QWidget* pWidget )
	{
		if (pWidget)
		{
			//用于设置子窗口的边框
			QWidget* pNestWidget = new QWidget(this);
			pNestWidget->show();

			QGridLayout* pGLayout = new QGridLayout(pNestWidget);
			pGLayout->setMargin(1);
			pGLayout->addWidget(pWidget);
		}
	}

	void CFreeSceneWidget::RemoveSubWidget( QWidget* pWidget )
	{
		QWidget* pParentWidget = GetParentWidget(pWidget);
		if (pParentWidget)
		{
			delete pParentWidget;
			pParentWidget = NULL;
		}
	}

	void CFreeSceneWidget::SetWidgetVisible( QWidget* pWidget, bool bVisible )
	{
		QWidget* pParentWidget = GetParentWidget(pWidget);
		if (pParentWidget)
		{
			if (bVisible)
			{
				pParentWidget->show();
			}
			else
			{
				pParentWidget->hide();
			}
		}
	}

	QWidget* CFreeSceneWidget::GetParentWidget( QWidget* pWidget )
	{
		int nCount = count();
		for (int i = 0; i < nCount; i++)
		{
			QWidget* pChildWidget = widget(i);
			if (NULL == pChildWidget)
			{
				continue;
			}

			QLayout* pLayout = pChildWidget->layout();
			if (NULL == pLayout)
			{
				continue;
			}

			int nLayoutChilds = pLayout->count();
			for (int j = 0; j < nLayoutChilds; j++)
			{
				if (pWidget == pLayout->itemAt(j)->widget())
				{
					return pChildWidget;
				}
			}
		}

		return NULL;
	}

	void CFreeSceneWidget::paintEvent( QPaintEvent * event )
	{
		////绘制边框线
		//QPainter painter(this);
		////painter.setRenderHint(QPainter::Antialiasing, true);

		////设置画笔的颜色
		//QPen pen;
		//pen.setColor(g_colorFrameLine);
		//painter.setPen(pen);

		//int nCount = count();

		////如果当前窗口中只有一个子窗体，则直接绘制外边框
		//if(nCount <= 1)
		//{
		//	QPolygon polygon;
		//	polygon.push_back(QPoint(0, 0));
		//	polygon.push_back(QPoint(width()-1, 0));
		//	polygon.push_back(QPoint(width()-1, height()-1));
		//	polygon.push_back(QPoint(0, height()-1));
		//	polygon.push_back(QPoint(0, 0));
		//	painter.drawPolyline(polygon);
		//}
		////如果有多个子窗体，则绘制每个子窗体的外边框
		//else
		//{
		//	for (int i = 0; i < nCount; i++)
		//	{
		//		QWidget* pWidget = widget(i);
		//		if (NULL == pWidget)
		//		{
		//			continue;
		//		}

		//		if (pWidget)
		//		{
		//			QPolygon polygon;
		//			polygon.push_back(QPoint(pWidget->x(), pWidget->y()));
		//			polygon.push_back(QPoint(pWidget->x()+pWidget->width(),pWidget->y()));
		//			polygon.push_back(QPoint(pWidget->x()+pWidget->width(), pWidget->y()+pWidget->height()));
		//			polygon.push_back(QPoint(pWidget->x(), pWidget->y()+pWidget->height()));
		//			polygon.push_back(QPoint(pWidget->x(), pWidget->y()));
		//			painter.drawPolyline(polygon);
		//		}
		//	}
		//}
	}

	void CFreeSceneWidget::resizeEvent( QResizeEvent * event )
	{
		QSplitter::resizeEvent(event);

		for (int i = 0; i < count(); i++)
		{
			QWidget* pWidget = widget(i);
			if (pWidget)
			{
				QList<int> list;
				list.push_back(width()/count());
				list.push_back(width()/count());
				setSizes(list);
			}
		}
	}

}
