#include <FreeMainWindow.h>

#include <QMouseEvent>
#include <QEvent>		 

#include <FreeStyleSheet.h>

#define min(a,b) ((a)<(b)? (a) :(b))
#define max(a,b) ((a)>(b)? (a) :(b))

CFreeMainWindow::CFreeMainWindow(QWidget *parent)
	:QFrame(parent)
	,m_pLogoWidget(NULL)
	,m_pControlWidget(NULL)
	,m_bShowNormal(false)
	,m_nEdgeMargin(6)
	,m_bMousePress(false)
	,m_nMouseDir(NO_DIR)
	,m_rectDoubleClickedArea(0, 0, 0, 0)
	,m_rectMovableArea(0, 0, 0, 0)
{
	InitWidget();

	InitContext();
}

CFreeMainWindow::~CFreeMainWindow()
{

}

void CFreeMainWindow::InitWidget()
{
	this->setStyleSheet(GetMainWindowStyle());

	//主窗口垂直布局
	QVBoxLayout* pVLayoutMainWindow = new QVBoxLayout(this);
	pVLayoutMainWindow->setSpacing(2);
	pVLayoutMainWindow->setContentsMargins(0, 0, 0, 0);

	//整体标题栏，包括标题显示以及控制按钮
	QFrame* pTitleFrame = new QFrame(this);
	pTitleFrame->setMouseTracking(true);
	pTitleFrame->setStyleSheet(GetTitleFrameStyle());
	pTitleFrame->setFrameShape(QFrame::StyledPanel);
	pTitleFrame->setFrameShadow(QFrame::Raised);

	//标题显示框
	QHBoxLayout* pHLayoutTitleFrame = new QHBoxLayout(pTitleFrame);
	pHLayoutTitleFrame->setSpacing(2);
	pHLayoutTitleFrame->setContentsMargins(0, 0, 0, 0);

	//标题
	m_pLabelTitle = new QLabel(pTitleFrame);
	m_pLabelTitle->setStyleSheet(GetTitleLabelStyle());
	m_pLabelTitle->setAlignment(Qt::AlignCenter);
	pHLayoutTitleFrame->addWidget(m_pLabelTitle);

	//整体控制按钮布局
	QVBoxLayout* pVLayoutCtrl = new QVBoxLayout();
	pVLayoutCtrl->setSpacing(1);

	//三个控制按钮的水平布局
	QHBoxLayout* pHLayoutCtrlBtns = new QHBoxLayout();
	pHLayoutCtrlBtns->setSpacing(1);
	pHLayoutCtrlBtns->setContentsMargins(-1, 5, 5, -1);

	//最小化按钮
	m_pMiniPushBtn = new QPushButton(pTitleFrame);
	m_pMiniPushBtn->setStyleSheet(GetMiniPushBtnStyle());
	m_pMiniPushBtn->setFixedSize(30, 23);
	m_pMiniPushBtn->setFocusPolicy(Qt::ClickFocus);
	pHLayoutCtrlBtns->addWidget(m_pMiniPushBtn);

	//还原按钮
	m_pNormalPushBtn = new QPushButton(pTitleFrame);
	m_pNormalPushBtn->setStyleSheet(GetNormalPushBtnStyle());
	m_pNormalPushBtn->setFixedSize(30, 23);
	m_pNormalPushBtn->setFocusPolicy(Qt::ClickFocus);
	pHLayoutCtrlBtns->addWidget(m_pNormalPushBtn);

	//关闭按钮
	m_pClosePushBtn = new QPushButton(pTitleFrame);
	m_pClosePushBtn->setStyleSheet(GetClosePushBtnStyle());
	m_pClosePushBtn->setFixedSize(30, 23);
	m_pClosePushBtn->setFocusPolicy(Qt::ClickFocus);
	pHLayoutCtrlBtns->addWidget(m_pClosePushBtn);

	pHLayoutCtrlBtns->setStretch(0, 1);
	pHLayoutCtrlBtns->setStretch(1, 1);
	pHLayoutCtrlBtns->setStretch(2, 1);
	pVLayoutCtrl->addLayout(pHLayoutCtrlBtns);

	QSpacerItem* pVCtrlSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
	pVLayoutCtrl->addItem(pVCtrlSpacer);

	pHLayoutTitleFrame->addLayout(pVLayoutCtrl);
	pVLayoutMainWindow->addWidget(pTitleFrame);

	m_pSceneGridLayout = new QGridLayout();
	m_pSceneGridLayout->setHorizontalSpacing(0);
	m_pSceneGridLayout->setVerticalSpacing(0);
	m_pSceneGridLayout->setContentsMargins(4, -1, 4, 4);
	pVLayoutMainWindow->addLayout(m_pSceneGridLayout);

	setWindowFlags(Qt::FramelessWindowHint);
	setMouseTracking(true);

	m_pLabelTitle->setAttribute(Qt::WA_TranslucentBackground);
	m_pLabelTitle->setMouseTracking(true);
	m_pLabelTitle->setText(tr("西安恒歌数码科技有限责任公司"));

	this->setMinimumSize(QSize(1280, 720));
	pTitleFrame->setFixedHeight(50);
	SetMovableArea(QRect(0, 0, this->width(), 50));
	SetDoubleClickedArea(QRect(0, 0, this->width(), 50));

	m_pLogoWidget = new QWidget(this);
	QLabel* pLabelLogo = new QLabel(this);
	QGridLayout* layout = new QGridLayout(m_pLogoWidget);
	layout->setMargin(0);
	layout->addWidget(pLabelLogo);

	m_pLogoWidget->setFixedSize(131, 40);
	
#ifdef WIN32
	m_pLogoWidget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
#else
	m_pLogoWidget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
#endif
	
	m_pLogoWidget->setAttribute(Qt::WA_TranslucentBackground, true);
	m_pLogoWidget->setAutoFillBackground(false);
	m_pLogoWidget->setStyleSheet("background: transparent; border:none;border-image:url(:/images/logo.png);");
	RepaintWidget();
	m_pLogoWidget->show();

	m_pControlWidget = new CFreeControlWidget(this);
	m_pControlWidget->setFixedSize(300, 500);
	RepaintWidget();
	m_pControlWidget->show();
}

void CFreeMainWindow::InitContext()
{
	connect(m_pMiniPushBtn, SIGNAL(clicked()), this, SLOT(SlotMinisizeWidget()));
	connect(m_pNormalPushBtn, SIGNAL(clicked()), this, SLOT(SlotMaxisizeWidget()));
	connect(m_pClosePushBtn, SIGNAL(clicked()), this, SLOT(SlotCloseWidget()));
}

void CFreeMainWindow::SetControlWidgetSize( QSize size )
{
	if (m_pControlWidget)
	{
		m_pControlWidget->setFixedSize(size);
	}
}

void CFreeMainWindow::AddControlItem( QSpacerItem* pItem )
{
	if (m_pControlWidget && pItem)
	{
		m_pControlWidget->GetLayout()->addItem(pItem);
	}
}

void CFreeMainWindow::AddControlWidget( QWidget* pWidget )
{
	if (m_pControlWidget && pWidget)
	{
		m_pControlWidget->GetLayout()->addWidget(pWidget);
	}
}

QGridLayout* CFreeMainWindow::GetSceneLayout()
{
	return m_pSceneGridLayout;
}

void CFreeMainWindow::SetTitle( QString strTitle )
{
	m_pLabelTitle->setText(strTitle);
}

void CFreeMainWindow::ShowDialogMaxisize()
{
	m_bShowNormal = false;
	showMaximized();
	
	RepaintWidget();
}

void CFreeMainWindow::ShowDialogMinisize()
{
	showMinimized();
}

void CFreeMainWindow::ShowDialogNormal()
{
	m_bShowNormal = true;
	showNormal();
}

void CFreeMainWindow::CheckEdge()
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

void CFreeMainWindow::SlotCloseWidget()
{
	close();
}

void CFreeMainWindow::SlotMinisizeWidget()
{
	showMinimized();
}

void CFreeMainWindow::SlotMaxisizeWidget()
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

void CFreeMainWindow::mousePressEvent( QMouseEvent *event )
{
	if(event->button() == Qt::LeftButton)
	{
		m_bMousePress = true;

		//鼠标相对于窗体的位置
		m_pointLastMouse = event->pos();
	}
}

void CFreeMainWindow::mouseReleaseEvent( QMouseEvent *event )
{
	//设置鼠标为未被按下
	m_bMousePress = false;

	if (m_nMouseDir != NO_DIR)
	{
		CheckEdge();
	}
}

void CFreeMainWindow::mouseMoveEvent( QMouseEvent *event )
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

				RepaintWidget();
			}
		}
		else
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

			RepaintWidget();
		}
	}
	//当不拖动窗口、不改变窗口大小尺寸的时候  检测鼠标边缘
	else
	{
		CheckEdge();  
	}

	event->ignore();
}

void CFreeMainWindow::mouseDoubleClickEvent( QMouseEvent* event )
{
	if (PointInArea(m_rectDoubleClickedArea, event->pos()))
	{
		SlotMaxisizeWidget();
	}
}

void CFreeMainWindow::SetMovableArea( QRect rectArea )
{
	m_rectMovableArea = rectArea;
}

void CFreeMainWindow::SetDoubleClickedArea( QRect rectArea )
{
	m_rectDoubleClickedArea = rectArea;
}

bool CFreeMainWindow::PointInArea( const QRect& rectArea, const QPoint& point )
{
	if (point.x() >= rectArea.x() && point.x() <= (rectArea.x()+rectArea.width()) 
		&&point.y() >= rectArea.y() && point.y() <= (rectArea.y()+rectArea.height()))
	{
		return true;
	}

	return false;
}

void CFreeMainWindow::resizeEvent( QResizeEvent* event )
{
	SetMovableArea(QRect(0, 0, this->width(), 40));
	SetDoubleClickedArea(QRect(0, 0, this->width(), 40));

	RepaintWidget();
}

void CFreeMainWindow::moveEvent( QMoveEvent* event )
{
	RepaintWidget();
}

void CFreeMainWindow::showEvent(QShowEvent *event)
{
	if (m_pControlWidget)
	{
		m_pControlWidget->show();
	}

	if(m_pLogoWidget)
	{
		m_pLogoWidget->show();
	}

	QFrame::showEvent(event);
}

void CFreeMainWindow::hideEvent(QHideEvent *event)
{
	static bool g_bHidden = true;

	if (g_bHidden)
	{
		if (m_pControlWidget)
		{
			m_pControlWidget->hide();
		}

		if(m_pLogoWidget)
		{
			m_pLogoWidget->hide();
		}

		g_bHidden = false;
	}

	QFrame::hideEvent(event);
}

void CFreeMainWindow::RepaintWidget()
{
	if (m_pLogoWidget)
	{
		m_pLogoWidget->setGeometry(x()+width()-m_pLogoWidget->width()-10, y()+height()-m_pLogoWidget->height()-10, m_pLogoWidget->width(), m_pLogoWidget->height());
	}

	if(m_pControlWidget)
	{
		m_pControlWidget->setGeometry(x()+20, y()+(height()-m_pControlWidget->height())/2, m_pControlWidget->width(), m_pControlWidget->height());
	}
}
