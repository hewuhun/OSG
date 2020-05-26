#include <mainWindow/FreeFramelessWidget.h>

#include <QMouseEvent>
#include <QEvent>		 
#include <QGraphicsDropShadowEffect>

namespace FreeViewer
{
#define min(a,b) ((a)<(b)? (a) :(b))
#define max(a,b) ((a)>(b)? (a) :(b))


	CFreeFramelessWidget::CFreeFramelessWidget(QWidget *parent)
		:QWidget(parent)
		,m_bShowNormal(false)
		,m_nEdgeMargin(6)
		,m_bMousePress(false)
		,m_bResizeWidget(true)
		,m_nMouseDir(NO_DIR)
		,m_rectDoubleClickedArea(0, 0, 0, 0)
		,m_rectMovableArea(0, 0, 0, 0)
	{
		InitWidget();
	}

	CFreeFramelessWidget::~CFreeFramelessWidget()
	{

	}

	void CFreeFramelessWidget::InitWidget()
	{
		setWindowFlags(Qt::FramelessWindowHint);
		setMouseTracking(true);
	}

	void CFreeFramelessWidget::ShowDialogMaxisize()
	{
		m_bShowNormal = false;
		showMaximized();
	}

	void CFreeFramelessWidget::ShowDialogMinisize()
	{
		showMinimized();
	}

	void CFreeFramelessWidget::ShowDialogNormal()
	{
		m_bShowNormal = true;
		showNormal();
	}

	void CFreeFramelessWidget::CheckEdge()
	{
		//计算鼠标距离窗口上下左右有多少距离
		int nDeltaLeft = abs(cursor().pos().x() - frameGeometry().left());      
		int nDeltaRight = abs(cursor().pos().x() - frameGeometry().right());
		int nDeltaTop = abs(cursor().pos().y() - frameGeometry().top());
		int nDeltaBottom = abs(cursor().pos().y() - frameGeometry().bottom());

		QCursor tempCursor = cursor();

		if(nDeltaTop < m_nEdgeMargin)
		{                             
			//根据 边缘距离 分类改变尺寸的方向
			if(nDeltaLeft < m_nEdgeMargin)
			{
				m_nMouseDir = TOP_LEFT;
				tempCursor.setShape(Qt::SizeFDiagCursor);
			}
			else if(nDeltaRight < m_nEdgeMargin)
			{
				m_nMouseDir = TOP_RIGHT;
				tempCursor.setShape(Qt::SizeBDiagCursor);
			}
			else
			{
				m_nMouseDir = TOP;
				tempCursor.setShape(Qt::SizeVerCursor);
			}
		}
		else if(nDeltaBottom < m_nEdgeMargin)
		{
			if(nDeltaLeft < m_nEdgeMargin)
			{
				m_nMouseDir = BOTTOM_LEFT;
				tempCursor.setShape(Qt::SizeBDiagCursor);
			}
			else if(nDeltaRight < m_nEdgeMargin)
			{
				m_nMouseDir = BOTTOM_RIGHT;
				tempCursor.setShape(Qt::SizeFDiagCursor);
			}
			else
			{
				m_nMouseDir = BOTTOM;
				tempCursor.setShape(Qt::SizeVerCursor);
			}
		}
		else if(nDeltaLeft < m_nEdgeMargin)
		{
			m_nMouseDir = LEFT;
			tempCursor.setShape(Qt::SizeHorCursor);
		}
		else if(nDeltaRight < m_nEdgeMargin)
		{
			m_nMouseDir = RIGHT;
			tempCursor.setShape(Qt::SizeHorCursor);
		}
		else
		{
			m_nMouseDir = NO_DIR;
			tempCursor.setShape(Qt::ArrowCursor);
		}

		//重新设置鼠标样式
		setCursor(tempCursor);                    
	}

	void CFreeFramelessWidget::SlotCloseWidget()
	{
		close();
	}

	void CFreeFramelessWidget::SlotMinisizeWidget()
	{
		showMinimized();
	}

	void CFreeFramelessWidget::SlotMaxisizeWidget()
	{
		if (m_bShowNormal)
		{
			ShowDialogMaxisize();
		}
		else
		{
			ShowDialogNormal();
		}
	}

	void CFreeFramelessWidget::mousePressEvent( QMouseEvent *event )
	{
		if(event->button() == Qt::LeftButton)
		{
			m_bMousePress = true;

			//鼠标相对于窗体的位置
			m_pointLastMouse = event->pos();
		}
	}

	void CFreeFramelessWidget::mouseReleaseEvent( QMouseEvent *event )
	{
		//设置鼠标为未被按下
		m_bMousePress = false;

		if (m_nMouseDir != NO_DIR && m_bResizeWidget)
		{
			CheckEdge();
		}
	}

	void CFreeFramelessWidget::mouseMoveEvent( QMouseEvent *event )
	{
		if (false == m_bShowNormal)
		{
			return QWidget::mouseMoveEvent(event);
		}

		if (m_bMousePress && m_bShowNormal)
		{
			//如果鼠标不是放在边缘那么说明这是在拖动窗口
			if(m_nMouseDir == NO_DIR)
			{                             
				if (PointInArea(m_rectMovableArea, m_pointLastMouse))
				{
					move(event->globalPos() - m_pointLastMouse);
				}
			}
			else if ( m_bResizeWidget)
			{
				//窗口上下左右的值
				int nTop, nBottom, nLeft, nRight;                   
				nTop = frameGeometry().top();
				nBottom = frameGeometry().bottom();
				nLeft = frameGeometry().left();
				nRight = frameGeometry().right();

				//检测更改尺寸方向中包含的上下左右分量
				if(m_nMouseDir & TOP)
				{                               
					if(height() == minimumHeight())
					{
						nTop = min(event->globalY(), nTop);
					}
					else if(height() == maximumHeight())
					{
						nTop = max(event->globalY(),nTop);
					}
					else
					{
						nTop = event->globalY();
					}
				}
				else if(m_nMouseDir & BOTTOM)
				{
					if(height() == minimumHeight())
					{
						nBottom = max(event->globalY(),nTop);
					}
					else if(height() == maximumHeight())
					{
						nBottom = min(event->globalY(),nTop);
					}
					else
					{
						nBottom = event->globalY();
					}
				}

				if(m_nMouseDir & LEFT)
				{                        //检测左右分量
					if(width() == minimumWidth())
					{
						nLeft = min(event->globalX(),nLeft);
					}
					else if(width() == maximumWidth())
					{
						nLeft = max(event->globalX(),nLeft);
					}
					else
					{
						nLeft = event->globalX();
					}
				}
				else if(m_nMouseDir & RIGHT)
				{
					if(width() == minimumWidth())
					{
						nRight = max(event->globalX(),nRight);
					}
					else if(width() == maximumWidth())
					{
						nRight = min(event->globalX(),nRight);
					}
					else
					{
						nRight = event->globalX();
					}
				}

				setGeometry(QRect(QPoint(nLeft, nTop), QPoint(nRight, nBottom)));
			}
		}
		//当不拖动窗口、不改变窗口大小尺寸的时候  检测鼠标边缘
		else if (m_bResizeWidget)
		{
			CheckEdge();  
		}

		event->ignore();
	}

	void CFreeFramelessWidget::mouseDoubleClickEvent( QMouseEvent* event )
	{
		if (PointInArea(m_rectDoubleClickedArea, event->pos()))
		{
			SlotMaxisizeWidget();
		}
	}

	void CFreeFramelessWidget::SetMovableArea( QRect rectArea )
	{
		m_rectMovableArea = rectArea;
	}

	void CFreeFramelessWidget::SetDoubleClickedArea( QRect rectArea )
	{
		m_rectDoubleClickedArea = rectArea;
	}

	void CFreeFramelessWidget::SetResizeWidget( bool bState )
	{
		m_bResizeWidget = bState;
	}

	bool CFreeFramelessWidget::PointInArea( const QRect& rectArea, const QPoint& point )
	{
		if (point.x() >= rectArea.x() && point.x() <= (rectArea.x()+rectArea.width()) 
			&&point.y() >= rectArea.y() && point.y() <= (rectArea.y()+rectArea.height()))
		{
			return true;
		}

		return false;
	}

}


