#include <mainWindow/FreeToolBar.h>

#include <mainWindow/FreeUtil.h>
#include <mainWindow/FreeToolButton.h>

#include <QFrame>
#include <QFile>
#include <QEvent>
#include <QMouseEvent>
#include <QWidgetAction>

namespace FreeViewer
{
	CFreeToolBar::CFreeToolBar( QWidget* pParent )
		:QWidget(pParent)
		,m_bMousePressed(false)
		,m_nDragDeltaX(0)
		,m_nTotalWidth(0)
		,m_bLooping(true)
	{
		InitWidget();

		InitContext();
	}

	CFreeToolBar::~CFreeToolBar()
	{

	}

	void CFreeToolBar::InitWidget()
	{
		SetStyleSheet(this, ":/css/freetoolbar.qss");
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}

	void CFreeToolBar::InitContext()
	{

	}

	void CFreeToolBar::mousePressEvent( QMouseEvent * event )
	{
		if(event->button() == Qt::LeftButton)
		{
			m_bMousePressed = true;
			m_lastPoint = event->globalPos();
		}
	}

	void CFreeToolBar::mouseMoveEvent( QMouseEvent * event )
	{
		if (m_bMousePressed)
		{
			//循环拖动
			if (m_bLooping)
			{
				RepaintLoopWidgets(event->globalPos());
			}
			//非循环拖动
			else
			{
				RepaintNoLoopWidgets(event->globalPos());
			}
		}
	}

	void CFreeToolBar::mouseReleaseEvent( QMouseEvent * event )
	{
		m_bMousePressed = false;
	}

	void CFreeToolBar::resizeEvent( QResizeEvent * event )
	{
		//将拖拽模式设置为循环拖拽，后面将判断是否为非循环模式
		m_bLooping = true;

		RecalcTotalWidth();

		//如果总长度小于显示窗口的大小，则不需要进行拖动
		if (m_nTotalWidth < width())
		{
			m_nDragDeltaX = 0;
			m_bLooping = false;
		}
		//或者总长度减任一子部件的长度均大于显示对话框的总长度，则满足使用循环拖动的条件
		else
		{
			for (int i = 0; i < m_listWidgets.size(); i++)
			{
				if ((m_nTotalWidth - m_listWidgets.at(i)->width()) < width())
				{
					m_nDragDeltaX = 0;
					m_bLooping = false;
				}
			}
		}

		//更新部件的位置
		//if (false == m_bLooping)
		//{
			RepaintWidgets();
		//}
	}

	void CFreeToolBar::RecalcTotalWidth()
	{
		m_nTotalWidth = 0;
		for (int i = 0; i < m_listWidgets.size(); i++)
		{
			QWidget* pWidget = m_listWidgets.at(i);
			m_nTotalWidth += pWidget->width();
			m_nTotalWidth += g_nToolBtnSpacing;
		}
	}

	void CFreeToolBar::RepaintWidgets()
	{
		//开始绘制的位置
		int nPosX = 0;

		for (int i = 0; i < m_listWidgets.size(); i++)
		{
			QWidget* pWidget = m_listWidgets.at(i);
			if (NULL != pWidget)
			{
				pWidget->setGeometry(
					nPosX, 
					0,
					pWidget->width(), 
					pWidget->height());
			}

			//下一个部件的起始位置需要叠加当前的部件的宽度和Spacing
			nPosX += pWidget->width() + g_nToolBtnSpacing;
		}
	}

	void CFreeToolBar::RepaintLoopWidgets(QPoint currentPos)
	{
		int nTempDeltaX = m_nDragDeltaX + (currentPos - m_lastPoint).x();

		//如果当前已经拖动了大于一圈的距离，则将多余的圈数减掉
		if (abs(nTempDeltaX) > m_nTotalWidth)
		{
			nTempDeltaX %= m_nTotalWidth;
		}

		//保存此部件之前的所有部件的总长度
		int nBeforWidth = 0;
		for (int i = 0; i < m_listWidgets.size(); i++)
		{
			//当前部件的起始X位置
			int nTempPosX = nTempDeltaX + nBeforWidth;
			int nPosX = 0;

			//如果当前部件或者一部分在显示范围内
			if (
				(nTempPosX > 0 && nTempPosX < width())
				||
				(nTempPosX < 0 && nTempPosX+m_listWidgets.at(i)->width() > 0)
				)
			{
				nPosX = nTempPosX;
			}

			//如果当前的部件被从左侧拖出
			if (nTempPosX+m_listWidgets.at(i)->width() < 0)
			{
				nPosX = nTempPosX+m_nTotalWidth;
			}

			//如果当前的部件被从右侧侧拖出
			if (nTempPosX > width())
			{
				nPosX = nTempPosX-m_nTotalWidth;
			}

			//设置部件的位置
			m_listWidgets.at(i)->setGeometry(
				nPosX, 
				m_listWidgets.at(i)->y(), 
				m_listWidgets.at(i)->width(), 
				m_listWidgets.at(i)->height());

			nBeforWidth+=m_listWidgets.at(i)->width()+g_nToolBtnSpacing;

			//更新偏移的位置以及上一次鼠标点击的位置
			m_nDragDeltaX = nTempDeltaX;
			m_lastPoint = currentPos;
		}
	}

	void CFreeToolBar::RepaintNoLoopWidgets(QPoint currentPos)
	{
		int nTempDeltaX = m_nDragDeltaX + (currentPos - m_lastPoint).x();

		int nBeforWidth = 0;
		for (int i = 0; i < m_listWidgets.size(); i++)
		{
			int nPosX = nTempDeltaX+nBeforWidth;
			//阻止第一个部件向右侧移动
			if (i == 0)
			{
				if (nPosX > 0)
				{
					return ;
				}
			}

			//阻止最后一个部件向左移动
			if (m_nTotalWidth+nTempDeltaX < width())
			{
				return ;
			}

			//移动部件
			m_listWidgets.at(i)->setGeometry(
				nTempDeltaX+nBeforWidth, 
				m_listWidgets.at(i)->y(), 
				m_listWidgets.at(i)->width(), 
				m_listWidgets.at(i)->height());

			nBeforWidth+=m_listWidgets.at(i)->width()+g_nToolBtnSpacing;

			//更新偏移的位置以及上一次鼠标点击的位置
			m_nDragDeltaX = nTempDeltaX;
			m_lastPoint = currentPos;
		}
	}

	void CFreeToolBar::AddToolButton( CFreeToolButton* pToolBtn )
	{
		if (pToolBtn)
		{
			//监听子按钮的鼠标按下、移动以及释放事件
			pToolBtn->installEventFilter(this);
			
			pToolBtn->setParent(this);
			pToolBtn->setFixedSize(g_sizeToolButton);
			pToolBtn->setIconSize(g_sizeToolButton);
			pToolBtn->show();
			m_listWidgets.push_back(pToolBtn);
		}

		RepaintWidgets();
	}

	bool CFreeToolBar::RemoveToolButton( CFreeToolButton* pToolBtn )
	{
		WidgetLists::iterator it = m_listWidgets.begin();
		while(it != m_listWidgets.end())
		{
			if (pToolBtn == *(it))
			{
				m_listWidgets.erase(it);
			}
			it++;
		}
	
		return true;
	}

	void CFreeToolBar::AddSperator()
	{
		QFrame* pLine = new QFrame(this);
		pLine->setFixedSize(g_sizeToolSperator);
		pLine->setFrameShape(QFrame::VLine);
		pLine->setFrameShadow(QFrame::Plain);
		pLine->show();
		m_listWidgets.push_back(pLine);

		RepaintWidgets();
	}

	bool CFreeToolBar::eventFilter( QObject * object, QEvent* event )
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* pMouseEvent = (QMouseEvent*)(event);
			mousePressEvent(pMouseEvent);
		}

		if (event->type() == QEvent::MouseMove)
		{
			QMouseEvent* pMouseEvent = (QMouseEvent*)(event);
			mouseMoveEvent(pMouseEvent);
		}

		if (event->type() == QEvent::MouseButtonRelease)
		{
			QMouseEvent* pMouseEvent = (QMouseEvent*)(event);
			mouseReleaseEvent(pMouseEvent);
		}

		return false;
	}

}





