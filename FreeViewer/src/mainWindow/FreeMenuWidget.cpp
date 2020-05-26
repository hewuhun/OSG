#include <mainWindow/FreeMenuWidget.h>

#include <mainWindow/FreeUtil.h>
#include <mainWindow/FreeToolBar.h>
#include <mainWindow/FreeToolButton.h>

#include <QFile>
#include <QGridLayout>
#include <QDesktopWidget>
#include <QPushButton>
#include <QTextCodec>
#include <QToolButton>
#include <QLayoutItem>
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <iostream>
#include <math.h>

namespace FreeViewer
{
	CFreeMenuWidget::CFreeMenuWidget( QWidget* pParent )
		:QWidget(pParent)
		,m_pMenuBar(NULL)
		,m_pToolBar(NULL)
	{
		ui.setupUi(this);

		InitWidget();

		InitContext();
	}

	CFreeMenuWidget::~CFreeMenuWidget()
	{

	}

	void CFreeMenuWidget::InitWidget()
	{
		setAutoFillBackground(true);
		setMouseTracking(true);

		SetStyleSheet(this, ":/css/freemenuwidget.qss");

		//设置整个菜单栏的大小以及变化策略
		setGeometry(0, 0, GetScreenGeometry().width(), GetScreenGeometry().height());
		setFixedHeight(g_sizeMenu.height());
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

		//添加MenuLogo
		{
			//设置LogoWidget的大小以及背景图片
			ui.pFrameMenuLogo->setFixedWidth(g_sizeMenuLogo.width()/g_sizeDefaultScreen.width()*GetScreenGeometry().width());
			ui.pFrameMenuLogo->setStyleSheet("border-image: url(:/images/menu/menulogo_bg.png);border:0px;");

			//设置logo中图片的大小、位置以及背景图片
			ui.pLabelMenuLogoImg->setFixedSize(
				g_sizeMenuLogoImg.width()/g_sizeDefaultScreen.width()*GetScreenGeometry().width(),
				g_sizeMenuLogoImg.width()/g_sizeDefaultScreen.width()*GetScreenGeometry().width());

			ui.pLabelMenuLogoImg->setGeometry(
				g_nMenuLogoImgX/g_sizeDefaultScreen.width()*GetScreenGeometry().width(),
				(g_sizeMenuLogo.height()-ui.pLabelMenuLogoImg->height())/2.0,
				ui.pLabelMenuLogoImg->width(),
				ui.pLabelMenuLogoImg->height());

			ui.pLabelMenuLogoImg->setStyleSheet("border-image: url(:/images/menu/menulogoimg.png);");
			ui.pLabelMenuLogoImg->setText("");

			//设置logo中文本图片的大小、位置以及背景图片
			ui.pLabelMenuLogoText->setFixedSize(
				g_sizeMenuLogoText.width()/g_sizeDefaultScreen.width()*GetScreenGeometry().width(),
				g_sizeMenuLogoText.height()/g_sizeDefaultScreen.height()*GetScreenGeometry().height());
			ui.pLabelMenuLogoText->setGeometry(
				(g_nMenuLogoImgX+g_nMenuLogoSpace)/g_sizeDefaultScreen.width()*GetScreenGeometry().width()+ui.pLabelMenuLogoImg->width(),
				(g_sizeMenuLogo.height()-ui.pLabelMenuLogoText->height())/2.0,
				ui.pLabelMenuLogoImg->width(),
				ui.pLabelMenuLogoImg->height());

			ui.pLabelMenuLogoText->setStyleSheet("border-image: url(:/images/menu/menulogotext.png);");
			ui.pLabelMenuLogoText->setText("");
		}

		//添加菜单栏
		{
			QVBoxLayout* pMenuWidgetLayout = new QVBoxLayout();
			pMenuWidgetLayout->setContentsMargins(0, 3, 0, 0);
			pMenuWidgetLayout->setAlignment(Qt::AlignCenter);
			ui.pMenuWidget->setLayout(pMenuWidgetLayout);

			m_pMenuBar = new QMenuBar(this);
			m_pMenuBar->setSizePolicy(QSizePolicy::Expanding  , QSizePolicy::Expanding);
			pMenuWidgetLayout->addWidget(m_pMenuBar);

			//for (int i = 0; i < 10; i++)
			//{
			//	QTextCodec *codec = QTextCodec::codecForName("GBK");
			//	QTextCodec::setCodecForCStrings(codec);
			//	QMenu* pMenu = new QMenu(QString("文件(F)"), m_pMenuBar);

			//	//背景透明以及菜单风格设置
			//	pMenu->setWindowFlags(pMenu->windowFlags() | Qt::FramelessWindowHint); 
			//	pMenu->setAttribute(Qt::WA_TranslucentBackground); 

			//	QAction* pAction1 = new QAction(QIcon(":/images/menu/menulogo.png"), QString("新建"), pMenu);
			//	pAction1->setCheckable(true);
			//	pAction1->setIconVisibleInMenu(false);
			//	pAction1->setChecked(true);
			//	pMenu->addAction(pAction1);

			//	QAction* pAction2 = new QAction(QString("保存"), pMenu);
			//	pAction2->setCheckable(true);
			//	pAction2->setChecked(false);
			//	pMenu->addAction(pAction2);

			//	QAction* pAction3 = new QAction(QString("另存为"), pMenu);
			//	pAction3->setCheckable(true);
			//	pAction3->setChecked(true);
			//	pMenu->addAction(pAction3);

			//	m_pMenuBar->addMenu(pMenu);
			//}
		}

		//添加工具栏
		{
			m_pToolBar = new CFreeToolBar(this);
			ui.pToolWidgetLayout->setContentsMargins(6, 6, 90, 0);
			ui.pToolWidgetLayout->addWidget(m_pToolBar);

			//{
			//	QTextCodec *codec = QTextCodec::codecForName("GBK");
			//	QTextCodec::setCodecForCStrings(codec);

			//	for (int i = 0; i < 20; i++)
			//	{
			//		CFreeToolButton* pToolBtn = new CFreeToolButton(
			//			QString("Action_A"), 
			//			QString(":/images/icon/保存.png"),
			//			QString(":/images/icon/保存.png"),
			//			QString(":/images/icon/保存_press.png"), 
			//			QString(":/images/icon/保存.png"),
			//			m_pToolBar);
			//		pToolBtn->setFixedSize(g_sizeToolButton);
			//		pToolBtn->setIconSize(g_sizeToolButton);
			//		pToolBtn->setCheckable(true);
			//		pToolBtn->setChecked(false);

			//		//CFreeToolButton* pSubTB = pToolBtn->AddMenuAction(QString("场景信息"), 
			//		//	QString(":/images/icon/场景信息.png"),
			//		//	QString(":/images/icon/场景信息.png"),
			//		//	QString(":/images/icon/场景信息_press.png"), 
			//		//	QString(":/images/icon/场景信息.png"));

			//		//pToolBtn->AddMenuAction(QString("导航器"), 
			//		//	QString(":/images/icon/导航器.png"),
			//		//	QString(":/images/icon/导航器.png"),
			//		//	QString(":/images/icon/导航器_press.png"), 
			//		//	QString(":/images/icon/导航器.png"));

			//		//pToolBtn->AddMenuAction(QString("点标记"), 
			//		//	QString(":/images/icon/点标记.png"),
			//		//	QString(":/images/icon/点标记.png"),
			//		//	QString(":/images/icon/点标记_press.png"), 
			//		//	QString(":/images/icon/点标记.png"));
			//	
			//		m_pToolBar->AddToolButton(pToolBtn);
			//	}
			//	m_pToolBar->AddSperator();
			//}
		}

		//最小化、最大化、关闭按钮
		{
			ui.pCloseWidget->setFixedWidth(g_sizeMinCloseButton.width()*2+g_sizeRestoreButton.width());
			ui.pCloseWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

			CFreeToolButton* pMinToolBtn = new CFreeToolButton(
				QString("Min"), 
				QString(":/images/icon/close/minimize_normal.png"),
				QString(":/images/icon/close/minimize_hover.png"),
				QString(":/images/icon/close/minimize_press.png"),
				QString(":/images/icon/close/minimize_normal.png"),
				this);
			pMinToolBtn->setFixedSize(g_sizeMinCloseButton.width(), g_sizeMinCloseButton.height());
			pMinToolBtn->setIconSize(QSize(g_sizeMinCloseButton.width(), g_sizeMinCloseButton.height()));
			pMinToolBtn->setToolTip(tr("Min"));
			pMinToolBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
			pMinToolBtn->setMouseTracking(true);
			pMinToolBtn->setFocusPolicy(Qt::NoFocus);
			ui.pCloseToolBtnLayout->addWidget(pMinToolBtn);
			connect(pMinToolBtn, SIGNAL(clicked()), this, SLOT(SlotMinisizeWidget()));

			CFreeToolButton* pMaxToolBtn = new CFreeToolButton(
				QString("Max"), 
				QString(":/images/icon/close/restore_normal.png"),
				QString(":/images/icon/close/restore_hover.png"),
				QString(":/images/icon/close/restore_press.png"),
				QString(":/images/icon/close/restore_normal.png"),
				this);
			pMaxToolBtn->setFixedSize(g_sizeMinCloseButton.width(), g_sizeMinCloseButton.height());
			pMaxToolBtn->setIconSize(QSize(g_sizeMinCloseButton.width(), g_sizeMinCloseButton.height()));
			pMaxToolBtn->setToolTip(tr("Max"));
			pMaxToolBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
			pMaxToolBtn->setMouseTracking(true);
			pMaxToolBtn->setFocusPolicy(Qt::NoFocus);
			ui.pCloseToolBtnLayout->addWidget(pMaxToolBtn);
			connect(pMaxToolBtn, SIGNAL(clicked()), this, SLOT(SlotMaxisizeWidget()));

			CFreeToolButton* pCloseToolBtn = new CFreeToolButton(
				QString("Close"), 
				QString(":/images/icon/close/close_normal.png"),
				QString(":/images/icon/close/close_hover.png"),
				QString(":/images/icon/close/close_press.png"),
				QString(":/images/icon/close/close_normal.png"),
				this);
			pCloseToolBtn->setFixedSize(g_sizeMinCloseButton.width(), g_sizeMinCloseButton.height());
			pCloseToolBtn->setIconSize(QSize(g_sizeMinCloseButton.width(), g_sizeMinCloseButton.height()));
			pCloseToolBtn->setToolTip(tr("Close"));
			pCloseToolBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
			pCloseToolBtn->setMouseTracking(true);
			pCloseToolBtn->setFocusPolicy(Qt::NoFocus);
			ui.pCloseToolBtnLayout->addWidget(pCloseToolBtn);
			connect(pCloseToolBtn, SIGNAL(clicked()), this, SLOT(SlotCloseWidget()));
		}

		//设置跟踪鼠标移动事件，使得拖动边框能够改变窗体大小
		{
			ui.pLabelMenuLogoText->setMouseTracking(true);
			ui.pLabelMenuLogoImg->setMouseTracking(true);

			m_pToolBar->setMouseTracking(true);
			ui.pToolWidget->setMouseTracking(true);

			m_pMenuBar->setMouseTracking(true);
			ui.pMenuCloseWidget->setMouseTracking(true);
			ui.pMenuToolWidget->setMouseTracking(true);
			ui.pCloseWidget->setMouseTracking(true);
		}
	}

	void CFreeMenuWidget::InitContext()
	{

	}

	QVector<QPoint> CFreeMenuWidget::CalculateOutterPoints()
	{
		QRect rect = geometry();

		//偏移像素大小
		double dOutterDis = (height()-g_nToolBarHeight)/tan(g_dToolBarBorderAngle);
		QVector<QPoint> points;

		//绘制外圈边框
		//以左上点为第一个点，顺时针排序其他点
		points.push_back(QPoint(rect.x(), rect.y()));
		points.push_back(QPoint(width()-g_nDeltaWidth, rect.y()));
		points.push_back(QPoint(width()-g_nDeltaWidth, height()-g_nDeltaWidth)); 
		points.push_back(QPoint(width()-g_nDeltaWidth-g_nToolBarLeftDis, height()-g_nDeltaWidth));
		points.push_back(QPoint(width()-dOutterDis-g_nToolBarLeftDis, height()-g_nToolBarHeight)); 
		points.push_back(QPoint(ui.pFrameMenuLogo->width(), height()-g_nToolBarHeight));
		points.push_back(QPoint(ui.pFrameMenuLogo->width()-dOutterDis, height()-g_nDeltaWidth));
		points.push_back(QPoint(rect.x(), height()-g_nDeltaWidth)); 
		points.push_back(QPoint(rect.x(), rect.y()));	

		return points;
	}

	QVector<QPoint> CFreeMenuWidget::CalculateInnerPoints()
	{
		double dOutterDis = (height()-g_nToolBarHeight)/tan(g_dToolBarBorderAngle);
		double dInnerBottomDis = g_nToolBarLineDelta/sin(g_dToolBarBorderAngle);
		double dInnerTopDis = g_nToolBarLineDelta*tan(g_dToolBarBorderAngle/2.0);

		//绘制内圈边框
		QVector<QPoint> points;
		//左上点
		points.push_back(QPoint(ui.pFrameMenuLogo->width()+dInnerTopDis, 
			height()-g_nToolBarHeight+g_nToolBarLineDelta));
		//右上点
		points.push_back(QPoint(width()-dOutterDis-g_nToolBarLeftDis-dInnerTopDis, 
			height()-g_nToolBarHeight+g_nToolBarLineDelta));
		//右下点
		points.push_back(QPoint(width()-g_nToolBarLeftDis-dInnerBottomDis, 
			height()-g_nDeltaWidth));
		//左下点
		points.push_back(QPoint(ui.pFrameMenuLogo->width()-dOutterDis+dInnerBottomDis, 
			height()-g_nDeltaWidth));
		//左上点
		points.push_back(QPoint(ui.pFrameMenuLogo->width()+dInnerTopDis, 
			height()-g_nToolBarHeight+g_nToolBarLineDelta));

		return points;
	}

	void CFreeMenuWidget::paintEvent( QPaintEvent * event )
	{
		////重绘背景图片
		//QPalette palette;
		//QPixmap imgTitle = QPixmap(":/images/menu/menutitle.png");
		//palette.setBrush(QPalette::Background, QBrush(imgTitle.scaled(width(), height())));
		//setPalette(palette);

		//绘制内外边框的线
		QPainter painter(this);
		//painter.setRenderHint(QPainter::Antialiasing, true);

		QPen pen;
		pen.setColor(g_colorFrameLine);
		painter.setPen(pen);

		//绘制外边框线
		QVector<QPoint> pointsOutter = CalculateOutterPoints();
		QPolygon polyOutter(pointsOutter);
		painter.drawPolyline(polyOutter);

		//绘制内边框线
		QVector<QPoint> pointsInner = CalculateInnerPoints();
		QPolygon polyInner(pointsInner);
		painter.setBrush(QBrush(QImage(":/images/menu/toolbar.png")));
		painter.drawPolygon(polyInner);
	}

	void CFreeMenuWidget::SlotMinisizeWidget()
	{
		emit SignalMinisizeWidget();
	}

	void CFreeMenuWidget::SlotMaxisizeWidget()
	{
		emit SignalMaxisizeWidget();
	}

	void CFreeMenuWidget::SlotCloseWidget()
	{
		emit SignalCloseWidget();
	}

	QMenuBar* CFreeMenuWidget::GetMenuBar()
	{
		return m_pMenuBar;
	}

	CFreeToolBar* CFreeMenuWidget::GetToolBar()
	{
		return m_pToolBar;
	}

	QMenu* CFreeMenuWidget::GetOrCreateMenu(QString strName)
	{
		//如果MenuBar还未创建，则返回空
		if(NULL == m_pMenuBar)
		{
			return NULL;
		}

		//遍历MenuBar中的菜单项，判断是否已经创建此名称的Menu
		QAction* pAction = NULL;
		QList<QAction*> actions = m_pMenuBar->actions();
		for (int i = 0; i < actions.size(); i++)
		{
			pAction = actions.at(i);
			if (pAction && pAction->text() == strName)
			{
				break;
			}
			pAction = NULL;
		}

		//如果已经创建
		QMenu* pMenu = NULL;
		if (pAction)
		{
			pMenu = pAction->menu();
		}

		//如果没有创建，则新建一个Menu
		if (NULL == pMenu)
		{
			pMenu = new QMenu(strName, m_pMenuBar);

			//背景透明以及菜单风格设置
			pMenu->setWindowFlags(pMenu->windowFlags() | Qt::FramelessWindowHint); 
			pMenu->setAttribute(Qt::WA_TranslucentBackground); 
		}

		m_pMenuBar->addMenu(pMenu);

		return pMenu;
	}

	void CFreeMenuWidget::resizeEvent( QResizeEvent * event )
	{
		//重绘背景图片
		QPalette palette;
		QPixmap imgTitle = QPixmap(":/images/menu/menutitle.png");
		palette.setBrush(QPalette::Background, QBrush(imgTitle.scaled(width(), height())));
		setPalette(palette);

		////绘制内外边框的线
		//QPainter painter(this);
		////painter.setRenderHint(QPainter::Antialiasing, true);

		//QPen pen;
		//pen.setColor(g_colorFrameLine);
		//painter.setPen(pen);

		////绘制外边框线
		//QVector<QPoint> pointsOutter = CalculateOutterPoints();
		//QPolygon polyOutter(pointsOutter);
		//painter.drawPolyline(polyOutter);

		////绘制内边框线
		//QVector<QPoint> pointsInner = CalculateInnerPoints();
		//QPolygon polyInner(pointsInner);
		//painter.setBrush(QBrush(QImage(":/images/menu/toolbar.png")));
		//painter.drawPolygon(polyInner);
	}

}






