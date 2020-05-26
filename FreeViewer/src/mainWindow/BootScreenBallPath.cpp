#include "BootScreenBallPath.h"
#include <math.h>
namespace FreeViewer
{
	CBootScreenBallPath::CBootScreenBallPath(QWidget *parent)
		: QWidget(parent)
	{
		ComputePathPoint();
	}

	void CBootScreenBallPath::paintEvent(QPaintEvent *event)
	{
		if (!m_vecPathPoints.isEmpty())
		{
			DrawPath();
		}
	}

	int CBootScreenBallPath::ComputePathPoint()
	{
		if (!m_vecPathPoints.isEmpty())
		{
			m_vecPathPoints.clear();
		}

		double x = 0;
		double y = 0;
		double w = 140;
		double h = 70;
		double len = w/6; //将宽6等分
		QPointF tempPoint(0.0, 0.0);

		//第1个点
		double firstX = x + 4;
		double firstY = y + h/2;
		tempPoint.setX(firstX);
		tempPoint.setY(firstY);
		m_vecPathPoints.push_back(tempPoint);

		//第2个点
		double secondX = x + len;
		double secondY = y + 4;
		tempPoint.setX(secondX);
		tempPoint.setY(secondY);
		m_vecPathPoints.push_back(tempPoint);

		//第3个点
		double thirdX = x + len*2;
		double thirdY = y + h - 4;
		tempPoint.setX(thirdX);
		tempPoint.setY(thirdY);
		m_vecPathPoints.push_back(tempPoint);

		//第4个点
		double fourthX = x + len*3;
		double fourthY = y + 4;
		tempPoint.setX(fourthX);
		tempPoint.setY(fourthY);
		m_vecPathPoints.push_back(tempPoint);

		//第5个点
		double fifthX = x + len*4;
		double fifthY = y + h - 4;
		tempPoint.setX(fifthX);
		tempPoint.setY(fifthY);
		m_vecPathPoints.push_back(tempPoint);

		//第6个点
		double sixthX = x + len*5;
		double sixthY = y + 4;
		tempPoint.setX(sixthX);
		tempPoint.setY(sixthY);
		m_vecPathPoints.push_back(tempPoint);

		//第7个点
		double seventhX = x + w - 4;
		double seventhY = y + h/2;
		tempPoint.setX(seventhX);
		tempPoint.setY(seventhY);
		m_vecPathPoints.push_back(tempPoint);

		return 0;
	}

	int CBootScreenBallPath::DrawPath()
	{
		if (m_vecPathPoints.isEmpty())
		{
			return 0;
		}

		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, true); //反走样
		painter.setPen(QPen(QBrush(QColor(0, 160, 233)), 1.5, Qt::SolidLine));

		for (int i=0; i<(m_vecPathPoints.size() - 1); i++)
		{
			painter.drawLine(m_vecPathPoints.at(i), m_vecPathPoints.at(i+1));
		}

		return 0;
	}

	CBootScreenBallPath::~CBootScreenBallPath(void)
	{
	}
}
