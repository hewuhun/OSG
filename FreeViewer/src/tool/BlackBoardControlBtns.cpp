#include <tool/BlackBoardControlBtns.h>

#include <QColorDialog>

namespace FreeViewer
{
	CBlackBoardControlBtns::CBlackBoardControlBtns(QWidget *parent)
		: QWidget(parent)
		, m_pCanvas(NULL)
		, m_state(0)
	{
		ui.setupUi(this);
		
		setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		setAttribute(Qt::WA_TranslucentBackground, true);
		setAutoFillBackground(false);
		setStyleSheet("background: transparent; border:none;");

		InitWidget();
	}

	void CBlackBoardControlBtns::InitWidget()
	{
		connect(ui.btn_line, SIGNAL(clicked(bool)), this, SLOT(SlotOnLineStripBtnClicked(bool)));
		connect(ui.btn_circle, SIGNAL(clicked(bool)), this, SLOT(SlotOnCircleBtnClicked(bool)));
		connect(ui.btn_rect, SIGNAL(clicked(bool)), this, SLOT(SlotOnRectBtnClicked(bool)));
		connect(ui.btn_mouse, SIGNAL(clicked(bool)), this, SLOT(SlotOnMousePathBtnClicked(bool)));
		connect(ui.btn_color, SIGNAL(clicked(bool)), this, SLOT(SlotOnColorBtnClicked(bool)));
		connect(ui.btn_clear, SIGNAL(clicked(bool)), this, SLOT(SlotOnClearBtnClicked(bool)));
		connect(ui.btn_exit, SIGNAL(clicked(bool)), this, SLOT(SlotOnExitBtnClicked(bool)));
	}

	void CBlackBoardControlBtns::SetBlackBoardWidget(CBlackBoard* pBlackBoard)
	{
		m_pBlackBoardWidget = pBlackBoard;
		connect(m_pBlackBoardWidget, SIGNAL(SignalBlackBoardDone(bool)), this, SLOT(SlotOnExitBtnClicked(bool)));

		m_pCanvas = m_pBlackBoardWidget->GetCanvas();
	}

	void CBlackBoardControlBtns::SlotOnClearBtnClicked(bool checked)
	{
		if(m_pCanvas)
		{
			m_pCanvas->ClearCanvas();
		}
	}

	void CBlackBoardControlBtns::SlotOnExitBtnClicked(bool checked)
	{
		if(m_pCanvas)
		{
			m_pCanvas->ClearCanvas();
		}

		m_pBlackBoardWidget->hide();
		this->hide();
	}

	void CBlackBoardControlBtns::SlotOnMousePathBtnClicked(bool checked)
	{
		if(m_pCanvas)
		{
			m_pCanvas->ActiveDrawUnit(new CMousePath(m_pCanvas->m_color));
			m_state =4;
		}
	}

	void CBlackBoardControlBtns::SlotOnLineStripBtnClicked(bool checked)
	{
		if(m_pCanvas)
		{
			m_pCanvas->ActiveDrawUnit(new CLine(m_pCanvas->m_color));
			m_state = 1;
		}
	}

	void CBlackBoardControlBtns::SlotOnCircleBtnClicked(bool checked)
	{
		if(m_pCanvas)
		{
			m_pCanvas->ActiveDrawUnit(new CCircle(m_pCanvas->m_color));
			m_state = 2;
		}
	}

	void CBlackBoardControlBtns::SlotOnRectBtnClicked(bool checked)
	{
		if(m_pCanvas)
		{
			m_pCanvas->ActiveDrawUnit(new CRect(m_pCanvas->m_color));
			m_state = 3;
		}
	}

	void CBlackBoardControlBtns::SlotOnColorBtnClicked(bool checked)
	{
		if (m_pCanvas)
		{
			m_pCanvas->m_color = QColorDialog::getColor(Qt::yellow);
			//点击当前绘制的图形按钮 触发颜色改变
			switch (m_state)
			{
			case 1: SlotOnLineStripBtnClicked(true);	break;
			case 2: SlotOnCircleBtnClicked(true);		break;
			case 3: SlotOnRectBtnClicked(true);			break;
			case 4: SlotOnMousePathBtnClicked(true);	break;
			default: break;
			}
		}
	}

	CBlackBoardControlBtns::~CBlackBoardControlBtns()
	{

	}
}

