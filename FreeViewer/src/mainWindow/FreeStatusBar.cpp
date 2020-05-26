#include <mainWindow/FreeStatusBar.h>

#include <mainWindow/FreeUtil.h>

#include <QSpacerItem>
#include <QPainter>

namespace FreeViewer
{
	CFreeStatusBar::CFreeStatusBar( QWidget* pParent )
		:QWidget(pParent)
		,m_pLayout(NULL)
		,m_pMessLabel(NULL)
		,m_pViewLable(NULL)
	{
		InitWidget();

		InitContext();
	}

	CFreeStatusBar::~CFreeStatusBar()
	{

	}

	void CFreeStatusBar::InitWidget()
	{
		setMouseTracking(true);
		setWindowFlags(Qt::FramelessWindowHint);
		SetStyleSheet(this, ":/css/freestatusbar.qss");

		m_pLayout = new QHBoxLayout();
		setLayout(m_pLayout);

		m_pViewLable = new QLabel();
		m_pViewLable->setText("");

		m_pMessLabel = new QLabel();
		//x00038 隐藏no error
		//m_pMessLabel->setText(tr("No Error"));
		QSpacerItem* pSpacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);

		m_pLayout->addWidget(m_pMessLabel);
		m_pLayout->addItem(pSpacer);
		m_pLayout->addWidget(m_pViewLable);
	}

	void CFreeStatusBar::InitContext()
	{

	}

	void CFreeStatusBar::paintEvent( QPaintEvent * event )
	{
		//绘制上边线
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, true);

		//设置画笔的颜色
		QPen pen;
		pen.setColor(g_colorFrameLine);
		painter.setPen(pen);

		//绘制线
		painter.drawLine(QPoint(0, 0), QPoint(width(), 0));
	}

	void CFreeStatusBar::SetMessageText( QString strText )
	{
		if (m_pMessLabel)
		{
			m_pMessLabel->setText(strText);
		}
	}

	void CFreeStatusBar::SetViewText( QString strLon, QString strLat, QString strHei, QString strCamHei )
	{
		if (m_pViewLable)
		{
			QString strText = tr("Longitude:").append(strLon).append("°  ")
				.append(tr("Latitude:")).append(strLat).append("°  ")
				.append(tr("Height:")).append(strHei).append("m  ")
				.append(tr("CamHei:")).append(strCamHei).append("m  ");

			m_pViewLable->setText(strText);
		}
	}
}





