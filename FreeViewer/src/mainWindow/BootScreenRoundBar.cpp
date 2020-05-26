#include "BootScreenRoundBar.h"

namespace FreeViewer
{
	CBootScreenRoundBar::CBootScreenRoundBar(QWidget *parent) :
	QWidget(parent)
		, m_min(0), m_max(100)
		, m_value(25)
		, m_nullPosition(PositionTop)
		, m_barStyle(StyleDonut)
		, m_outlinePenWidth(1)
		, m_dataPenWidth(1)
		, m_rebuildBrush(false)
		, m_format("%p%")
		, m_decimals(1)
		, m_updateFlags(UF_PERCENT)
	{
		m_pLabel = new QLabel("%", this);
		m_pLabel->setGeometry(120, 50, 50, 50);
		m_pLabel->setStyleSheet("color: rgb(255, 255, 255); font: 75 20pt; font-family: Microsoft YaHei;");
	}
	
	void CBootScreenRoundBar::setRange(double min, double max)
	{
		m_min = min;
		m_max = max;
	
		if (m_max < m_min)
			qSwap(m_max, m_min);
	
		if (m_value < m_min)
			m_value = m_min;
		else if (m_value > m_max)
			m_value = m_max;
	
		if (!m_gradientData.isEmpty())
			m_rebuildBrush = true;
	
		update();
	}
	
	void CBootScreenRoundBar::setMinimum(double min)
	{
		setRange(min, m_max);
	}
	
	void CBootScreenRoundBar::setMaximum(double max)
	{
		setRange(m_min, max);
	}
	
	void CBootScreenRoundBar::setValue(double val)
	{
		if (m_value != val)
		{
			if (val < m_min)
				m_value = m_min;
			else if (val > m_max)
				m_value = m_max;
			else
				m_value = val;
	
			update();
		}
	}
	
	void CBootScreenRoundBar::setValue(int val)
	{
		setValue(double(val));
	}
	
	void CBootScreenRoundBar::setBarStyle(CBootScreenRoundBar::BarStyle style)
	{
		if (style != m_barStyle)
		{
			m_barStyle = style;
	
			update();
		}
	}
	
	void CBootScreenRoundBar::setOutlinePenWidth(double penWidth)
	{
		if (penWidth != m_outlinePenWidth)
		{
			m_outlinePenWidth = penWidth;
	
			update();
		}
	}
	
	void CBootScreenRoundBar::setDataPenWidth(double penWidth)
	{
		if (penWidth != m_dataPenWidth)
		{
			m_dataPenWidth = penWidth;
	
			update();
		}
	}
	
	void CBootScreenRoundBar::setDataColors(const QGradientStops &stopPoints)
	{
		if (stopPoints != m_gradientData)
		{
			m_gradientData = stopPoints;
			m_rebuildBrush = true;
	
			update();
		}
	}
	
	void CBootScreenRoundBar::setFormat(const QString &format)
	{
		if (format != m_format)
		{
			m_format = format;
	
			valueFormatChanged();
		}
	}
	
	void CBootScreenRoundBar::resetFormat()
	{
		m_format = QString::null;
	
		valueFormatChanged();
	}
	
	void CBootScreenRoundBar::setDecimals(int count)
	{
		if (count >= 0 && count != m_decimals)
		{
			m_decimals = count;
	
			valueFormatChanged();
		}
	}
	
	void CBootScreenRoundBar::paintEvent(QPaintEvent* event)
	{
		double outerRadius = qMin(width(), height());
		QRectF baseRect(1, 1, outerRadius-2, outerRadius-2);
	
		QImage buffer(outerRadius, outerRadius, QImage::Format_ARGB32_Premultiplied);
		buffer.fill(QColor(0, 0, 0, 0));
	
		QPainter p(&buffer);
		p.setRenderHint(QPainter::Antialiasing, true); //反走样
	
		// data circle
		double arcStep = 360.0 / (m_max - m_min) * m_value;
		drawValue(p, baseRect, m_value, arcStep);
	
		// center circle
		double innerRadius(0);
		QRectF innerRect;
		calculateInnerRect(baseRect, outerRadius, innerRect, innerRadius);
		drawInnerBackground(p, innerRect);
	
		// text
		innerRect = QRectF(1, 15, 153, 153);
		drawText(p, innerRect, innerRadius, m_value);
	
		// finally draw the bar
		p.end();
	
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.drawImage(0,0, buffer);
	
		// draw out circle
		QPen pen; //画笔
		pen.setColor(QColor(41, 66, 92));
		painter.setPen(pen); //添加画笔
		int w = this->geometry().width();
		int h = this->geometry().height();
		QRectF rectangle(0, 0, w, h);
		painter.drawEllipse(rectangle);
	}
	
	void CBootScreenRoundBar::drawValue(QPainter &p, const QRectF &baseRect, double value, double arcLength)
	{
		// nothing to draw
		if (value == m_min)
			return;
	
		// for Line style
		if (m_barStyle == StyleLine)
		{
			p.setPen(QPen(palette().highlight().color(), m_dataPenWidth));
			p.setBrush(Qt::NoBrush);
			p.drawArc(baseRect.adjusted(m_outlinePenWidth/2, m_outlinePenWidth/2, -m_outlinePenWidth/2, -m_outlinePenWidth/2),
				m_nullPosition * 16,
				-arcLength * 16);
			return;
		}
	
		// for Pie and Donut styles
		QPainterPath dataPath;
		dataPath.setFillRule(Qt::WindingFill);
	
		// pie segment outer
		dataPath.moveTo(baseRect.center());
		dataPath.arcTo(baseRect, m_nullPosition, -arcLength);
		dataPath.lineTo(baseRect.center());
	
		p.setBrush(palette().highlight());
		p.setPen(QPen(palette().shadow().color(), m_dataPenWidth));
		p.drawPath(dataPath);
	}
	
	void CBootScreenRoundBar::calculateInnerRect(const QRectF &/*baseRect*/, double outerRadius, QRectF &innerRect, double &innerRadius)
	{
		// for Line style
		if (m_barStyle == StyleLine)
		{
			innerRadius = outerRadius - m_outlinePenWidth;
		}
		else    // for Pie and Donut styles
		{
			innerRadius = outerRadius * 0.75;
		}
	
		double delta = (outerRadius - innerRadius) / 2;
		innerRect = QRectF(delta, delta, innerRadius, innerRadius);
	}
	
	void CBootScreenRoundBar::drawInnerBackground(QPainter &p, const QRectF &innerRect)
	{
		if (m_barStyle == StyleDonut)
		{
			p.setBrush(palette().alternateBase());
			p.drawEllipse(innerRect);
		}
	}
	
	void CBootScreenRoundBar::drawText(QPainter &p, const QRectF &innerRect, double innerRadius, double value)
	{
		if (m_format.isEmpty())
			return;
	
		// !!! to revise
		QFont f(font());
		f.setPointSize(42);
		f.setFamily("msyh");
		f.setItalic(true);
		p.setFont(f);
	
		QRectF textRect(innerRect);
		p.setPen(palette().text().color());
		p.drawText(textRect, Qt::AlignCenter, valueToText(value));
	}
	
	QString CBootScreenRoundBar::valueToText(double value) const
	{
		QString textToDraw(m_format);
	
		if (m_updateFlags & UF_VALUE)
			textToDraw.replace("%v", QString::number(value, 'f', m_decimals));
	
		if (m_updateFlags & UF_PERCENT)
		{
			double procent = (value - m_min) / (m_max - m_min) * 100.0;
			textToDraw.replace("%p", QString::number(procent, 'f', m_decimals));
		}
	
		if (m_updateFlags & UF_MAX)
			textToDraw.replace("%m", QString::number(m_max - m_min + 1, 'f', m_decimals));
	
		return textToDraw;
	}
	
	void CBootScreenRoundBar::valueFormatChanged()
	{
		m_updateFlags = 0;
	
		if (m_format.contains("%v"))
			m_updateFlags |= UF_VALUE;
	
		if (m_format.contains("%p"))
			m_updateFlags |= UF_PERCENT;
	
		if (m_format.contains("%m"))
			m_updateFlags |= UF_MAX;
	
		update();
	}
}
