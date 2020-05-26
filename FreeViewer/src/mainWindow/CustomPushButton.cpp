#include <mainWindow/CustomPushButton.h>
#include <QPainter>
#include <QIcon>

const int nFontSize = 12;

namespace FreeViewer
{
	CCustomPushButton::CCustomPushButton(const QString &text, QWidget *parent)
		: QPushButton(text, parent)
		, m_text(text)
		, m_nTextW(0)
		, m_nTextH(0)
		, m_BtnStatus(NORMAL)
	{
		ComputeBtnTextNum();
	}

	void CCustomPushButton::ComputeBtnTextNum()
	{
		m_nTextW = 30;
		m_nTextH = m_text.count() * nFontSize * 2;
		
		this->setFixedSize(m_nTextW, m_nTextH);
	}

	void CCustomPushButton::paintEvent(QPaintEvent *event)
	{
		//字体旋转90度
		QPainter painter(this);
		QTransform transform;
		transform.rotate(90.0);
		painter.setWorldTransform(transform);
		painter.drawText(nFontSize, -8, m_text);

		//绘制边框
		QPainter paint(this);
		paint.setPen(QColor(44,80,114));
		int rectW = m_nTextW - 1;
		int rectH = m_nTextH - 1;
		paint.drawRect(0, 0, rectW, rectH);

		//绘制背景图片
		QPainter pain(this);
		QPixmap pixmap;
		switch (m_BtnStatus)
		{
		case NORMAL:
			break;

		case ENTER:
			pixmap.load(":/images/tree/vTitleBar_back_hover.png");
			break;

		default:
			break;
		}
		pain.drawPixmap(0, 0, rectW, rectH, pixmap);
	}

	void CCustomPushButton::mousePressEvent(QMouseEvent *e)
	{
		emit SignalBtnClicked(m_text);
	}

	void CCustomPushButton::enterEvent(QEvent *event)
	{
		m_BtnStatus = ENTER;

		update();
	}

	void CCustomPushButton::leaveEvent(QEvent *event)
	{
		m_BtnStatus = NORMAL;

		update();
	}

	CCustomPushButton::~CCustomPushButton(void)
	{
	}
}
