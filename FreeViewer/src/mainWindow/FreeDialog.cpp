#include "FreeDialog.h"
#include <mainWindow/FreeUtil.h>
#include <QApplication>
#include <QDesktopWidget>

namespace FreeViewer
{
	CFreeDialog::CFreeDialog(QWidget *parent)
		: CFreeFramelessWidget(parent)
	{
		InitData();
	}

	void CFreeDialog::InitData()
	{
		//对话框最前显示
		this->setWindowFlags(windowFlags() | Qt::Tool);
		//设置窗体透明度
		this->setWindowOpacity(0.9);

		m_pTitleWidget = new QWidget(this);
		m_pTitleIcon = new QLabel(this);
		m_pTitleText = new QLabel(this);

		m_pCloseBtn = new QPushButton(this);

		//设置标题栏
		SetTitleWidget();

		//设置样式表
		SetStyleSheet(this, ":/css/freedialog.qss");

		//关联关闭按钮信号和槽
		connect(m_pCloseBtn, SIGNAL(clicked()), this, SLOT(SlotCloseBtnClicked()));
		
	}

	void CFreeDialog::SetTitleWidget()
	{
		//设置标题栏背景
		int nTitleWidgetH = 30;
		m_pTitleWidget->setFixedHeight(nTitleWidgetH);
		m_pTitleWidget->setStyleSheet("border: 1px solid rgb(44,80,114); border-bottom: none;"
									"background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0,"
									"stop:0 rgba(16, 30, 43, 255), stop:1 rgba(32, 62, 90, 255));");

		//设置标题栏图标
		int nTitleIconW = 28;
		int nTitleIconH = 28;
		m_pTitleIcon->setFixedSize(nTitleIconW, nTitleIconH);
		m_pTitleIcon->setStyleSheet("background: transparent; border: none; image: url(:/images/dialog/titleIcon.png);");

		//设置标题栏文本
		int nTitleTextH = 28;
		m_pTitleText->setFixedHeight(nTitleTextH);
		m_pTitleText->setStyleSheet("background: transparent; border: none;");

		//设置关闭按钮
		int nCloseBtnW = 28;
		int nCloseBtnH = 28;
		m_pCloseBtn->setFixedSize(nCloseBtnW, nCloseBtnH);
		m_pCloseBtn->setStyleSheet("QPushButton{border-image: url(:/images/dialog/closeBtn.png);}"
			"QPushButton:hover{ border-image: url(:/images/dialog/closeBtn_hover.png);}"
			"QPushButton:pressed{ border-image: url(:/images/dialog/closeBtn_pressed.png);}");

		//标题栏添加布局
		QHBoxLayout* pHLayout = new QHBoxLayout();
		pHLayout->setContentsMargins(1, 0, 1, 0);
		pHLayout->setSpacing(0);
		m_pTitleWidget->setLayout(pHLayout);

		pHLayout->addWidget(m_pTitleIcon);
		pHLayout->addWidget(m_pTitleText);
		pHLayout->addStretch();
		pHLayout->addWidget(m_pCloseBtn);

		//对话框整体布局
		m_pVLayout = new QVBoxLayout();
		m_pVLayout->setContentsMargins(0, 0, 0, 0);
		m_pVLayout->setSpacing(0);

		//添加标题栏到整体布局
		m_pVLayout->addWidget(m_pTitleWidget);

		//设置整体布局
		setLayout(m_pVLayout);

		//设置可拖动窗口的区域
		SetMovableArea(QRect(0, 0, width(), 30));

		//设置不可改变大小
		SetResizeWidget(false);
	}

	void CFreeDialog::SetTitleText(QString text)
	{
		QString str = tr("LineNode");
		m_pTitleText->setText(tr("%1").arg(text));
	}

	void CFreeDialog::AddWidgetToDialogLayout(QWidget *widget)
	{
		m_pVLayout->addWidget(widget);
	}

	void CFreeDialog::SlotCloseBtnClicked()
	{
		this->close();
	}

	void CFreeDialog::resizeEvent(QResizeEvent* event)
	{
		SetMovableArea(QRect(0, 0, width(), 30));

                QDesktopWidget* desktop = QApplication::desktop();
                move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);
	}

	void CFreeDialog::mouseDoubleClickEvent(QMouseEvent* event)
	{
		//禁用鼠标双击事件
	}

	CFreeDialog::~CFreeDialog(void)
	{
		if (m_pTitleIcon)
		{
			delete m_pTitleIcon;
			m_pTitleIcon = NULL;
		}

		if (m_pTitleText)
		{
			delete m_pTitleText;
			m_pTitleText = NULL;
		}

		if (m_pCloseBtn)
		{
			delete m_pCloseBtn;
			m_pCloseBtn = NULL;
		}

		if (m_pTitleWidget)
		{
			delete m_pTitleWidget;
			m_pTitleWidget = NULL;
		}

		if (m_pVLayout)
		{
			delete m_pVLayout;
			m_pVLayout = NULL;
		}
	}
}
