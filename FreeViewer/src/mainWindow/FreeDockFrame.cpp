#include "FreeDockFrame.h"
#include "qfile.h"

#include <mainWindow/FreeUtil.h>
#include <mainWindow/CustomPushButton.h>

namespace FreeViewer
{
	CDockFrame::CDockFrame(QWidget *parent) 
		: QWidget(parent)
	{
		ui.setupUi(this);

		//设置样式表
		SetStyleSheet(this, ":/css/docktree.qss");

		InitData();
	}

	CDockFrame::~CDockFrame()
	{

	}

	void CDockFrame::SetFixedWidth( int nTotalWidth, int nTitleWidth )
	{
		m_nWinWidth = nTotalWidth;
		m_nTitleBarWidth = nTitleWidth;

		setFixedWidth(m_nWinWidth);
	}

	void CDockFrame::InitData()
	{
		m_nWinWidth = 250;
		m_nTitleBarWidth = 30;

		this->setWindowFlags(Qt::FramelessWindowHint); //设置无边框
		this->setAttribute(Qt::WA_TranslucentBackground);
		this->setFixedWidth(m_nWinWidth);

		m_nDockCount = 0;
		m_nDockHideCount = 0;

		//实例化竖向标题栏
		m_pWidgetTitleBar = new QWidget(this);
		m_pWidgetTitleBar->setFixedWidth(m_nTitleBarWidth);
		m_pWidgetTitleBar->setStyleSheet("QWidget { background: transparent; border: 1px solid rgb(44,80,114);}");

		//实例化竖向标题栏布局
		m_pVLayout = new QVBoxLayout();
		m_pVLayout->setContentsMargins(0, 0, 0, 0);
		m_pVLayout->setSpacing(0);
		m_pVLayout->addStretch();
		m_pWidgetTitleBar->setLayout(m_pVLayout);

		ui.vLayout_addTitleBar->addWidget(m_pWidgetTitleBar);

		//初始化隐藏标题栏
		m_pWidgetTitleBar->hide();
	}

	QWidget* CDockFrame::GetTitleBar()
	{
		return m_pWidgetTitleBar;
	}

	void CDockFrame::AddDockWidget(CFreeDockWidget *dockWidget)
	{
		if (dockWidget == NULL)
		{
			return;
		}

		//保存悬浮窗
		m_vecDocks.append(dockWidget);

		//累计创建悬浮窗的数
		m_nDockCount++;

		//添加悬浮窗到布局
		ui.vLayout_addDocks->addWidget((QWidget*)dockWidget);

		//关联悬浮窗隐藏时标题文本的信号和槽
		connect(dockWidget, SIGNAL(SignalSendTitleText(QString)), SLOT(SlotReceiveTitleText(QString)));
	}

	void CDockFrame::AddPushBtnToLayout(QPushButton *pushBtn)
	{
		m_VecBtns.push_back(pushBtn);
		m_pVLayout->insertWidget(0, pushBtn);

		//关联标题栏按钮选中信号和槽
		connect(pushBtn, SIGNAL(SignalBtnClicked(QString)), this, SLOT(SlotClickedTitleBar(QString)));
	}

	void CDockFrame::CreateTitleBar()
	{
		if (m_titleList.isEmpty())
		{
			return;
		}

		QString strTitle = m_titleList.last();
		CCustomPushButton *pTitleBtn = new CCustomPushButton(strTitle);
		pTitleBtn->setMouseTracking(true);
		AddPushBtnToLayout(pTitleBtn);

		//设置标题栏宽度
		m_pWidgetTitleBar->setFixedWidth(m_nTitleBarWidth);
		m_pWidgetTitleBar->show();
	}

	void CDockFrame::SlotReceiveTitleText(const QString str)
	{
		m_nDockHideCount++;

		m_titleList.push_back(str);
		CreateTitleBar();

		if (m_nDockHideCount >= m_nDockCount)
		{
			this->setFixedWidth(m_nTitleBarWidth);
		}
	}

	void CDockFrame::SlotClickedTitleBar(QString text)
	{
		if (m_titleList.last().isEmpty())
		{
			return;
		}
		else
		{
			m_nDockHideCount--;
			if (m_nDockHideCount < 0)
			{
				m_nDockHideCount = 0;
				return;
			}

			//比较标题框文本，获取在m_titleList中的位置
			int listNum = -1;
			for (int i=0; i<m_titleList.size(); i++)
			{
				if (text == m_titleList.at(i))
				{
					listNum = i;
					break;
				}
			}

			//移除当前选中的标题框
			if (listNum >= 0 && listNum < m_titleList.size())
			{
				m_titleList.removeAt(listNum);

				QPushButton* btn = m_VecBtns.at(listNum);
				delete btn;
				btn = NULL;
				m_VecBtns.remove(listNum);
			}

			//比较标题框文本，获取在m_vecDocks中的位置
			int dockNum = -1;
			for (int j=0; j<m_vecDocks.size(); j++)
			{
				QString titleText = m_vecDocks.at(j)->ui.label_titleText->text();
				if (text == titleText)
				{
					dockNum = j;
					break;
				}
			}

			if (dockNum >= 0 && dockNum < m_vecDocks.size())
			{
				if (m_nDockHideCount == 0)
				{
					m_pWidgetTitleBar->hide();
				}
				//显示选择标题栏对应的悬浮窗
				m_vecDocks.at(dockNum)->show();
				this->setFixedWidth(m_nWinWidth);
			}

		}
	}

}
