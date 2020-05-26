#include <FreeControlWidget.h>

#include <FreeStyleSheet.h>

CFreeControlWidget::CFreeControlWidget(QWidget *parent)
	:QWidget(parent)
	,m_pVLayout(NULL)
{
	InitWidget();

	InitContext();
}

CFreeControlWidget::~CFreeControlWidget()
{

}

void CFreeControlWidget::InitWidget()
{
#ifdef WIN32
	setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
#else
	setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
#endif
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAutoFillBackground(false);
	setStyleSheet(QString("QWidget{background: transparent; border:none;}"));

	QFrame* pFrame = new QFrame(this);
	QGridLayout* pLayout = new QGridLayout(this);
	pLayout->setMargin(0);
	pLayout->setSpacing(0);
	pLayout->addWidget(pFrame);
	pFrame->setObjectName("FrameControlWidget");
	pFrame->setStyleSheet(GetControlWidgetStyle());

	m_pVLayout = new QVBoxLayout(pFrame);
	m_pVLayout->setContentsMargins(25, 20, 15, 5);
	m_pVLayout->setSpacing(15);
}

void CFreeControlWidget::InitContext()
{

}

QVBoxLayout* CFreeControlWidget::GetLayout()
{
	return m_pVLayout;
}




CFreeLableWidget::CFreeLableWidget(QWidget* parent)
	:QWidget(parent)
	,m_pTitleLabel(NULL)
	,m_pContentLabel(NULL)
{
	InitWidget();
}

CFreeLableWidget::~CFreeLableWidget()
{

}

void CFreeLableWidget::SetTitleText( QString strTitle )
{
	if (m_pTitleLabel)
	{
		m_pTitleLabel->setText(strTitle);
	}
}

void CFreeLableWidget::SetContentText( QString strContent )
{
	if (strContent.isEmpty())
	{
		return;
	}

	if (NULL == m_pContentLabel)
	{
		m_pContentLabel = new QLabel(this);
		m_pContentLabel->adjustSize();	//自适应大小
		m_pContentLabel->setWordWrap(true);	//自动换行
		m_pContentLabel->setStyleSheet(GetLabelContentStyle());
		layout()->addWidget(m_pContentLabel);
	}

	if (m_pContentLabel)
	{
		m_pContentLabel->setText(strContent);
	}
}

void CFreeLableWidget::InitWidget()
{
	QVBoxLayout * pLabelVLayout = new QVBoxLayout(this);
	pLabelVLayout->setSpacing(10);
	pLabelVLayout->setContentsMargins(0, 0, 0, 0);

	m_pTitleLabel = new QLabel(this);
	m_pTitleLabel->setAlignment(Qt::AlignCenter);
	m_pTitleLabel->setStyleSheet(GetLabelTitleStyle());
	pLabelVLayout->addWidget(m_pTitleLabel);
}
