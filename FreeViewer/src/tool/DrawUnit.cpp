#include <tool/DrawUnit.h>

namespace FreeViewer
{
	QRect CreateRect(QPoint pStart, QPoint pEnd)
	{
		QPoint minPoint(pStart);
		QPoint maxPoint(pEnd);

		if(pStart.x() > pEnd.x())
		{
			minPoint.setX(pEnd.x());
			maxPoint.setX(pStart.x());
		}

		if(pStart.y() > pEnd.y())
		{
			minPoint.setY(pEnd.y());
			maxPoint.setY(pStart.y());
		}

		return QRect(minPoint, maxPoint);
	}

	////////////////////////////////////////////////////////////////Mouse Path
	CMousePath::CMousePath(QColor color)
		:CDrawUnit()
		 ,m_bIsPressed(false)
		 ,m_color(color)
	{
	
	}

	CMousePath::~CMousePath()
	{

	}

	void CMousePath::Render(QPainter* pPainter)
	{
		if(pPainter)
		{
			pPainter->setPen(QPen(m_color));
			for(PointList::iterator itr = m_pointList.begin(); itr != m_pointList.end(); ++itr)
			{
				pPainter->setPen(QPen(points.m_color));
				LinePoints points = *itr;
				pPainter->drawLine(points.m_pStart, points.m_pEnd);
			}
		}

	}

	void CMousePath::mouseMoveEvent(QMouseEvent * pEvent)
	{
		if(m_bIsPressed)
		{
			m_endPoint = pEvent->pos();
			points.m_pStart = m_startPoint;
			points.m_pEnd = m_endPoint;
			points.m_color = m_color;
			m_pointList.push_back(points);
			m_startPoint = m_endPoint;
		}
	}

	void CMousePath::mousePressEvent(QMouseEvent * pEvent)
	{
		m_startPoint = pEvent->pos();
		m_endPoint = pEvent->pos();
		m_bIsPressed = true;
	}

	void CMousePath::mouseReleaseEvent(QMouseEvent * pEvent)
	{
		m_bIsPressed = false;
	}



	////////////////////////////////////////////////////////////////Lines
	CLine::CLine(QColor color)
		: CDrawUnit()
		, m_bIsPressed(false)
		, m_color(color)
	{
	
	}

	CLine::~CLine()
	{

	}

	void CLine::Render(QPainter* pPainter)
	{	
		if(pPainter)
		{
			pPainter->setPen(QPen(m_color));
			for(PointList::iterator itr = m_pointList.begin(); itr != m_pointList.end(); ++itr)
			{
				LinePoints points = *itr;
				pPainter->setPen(QPen(points.m_color));
				pPainter->drawLine(points.m_pStart, points.m_pEnd);
			}

			if(m_bIsPressed && (m_startPoint != m_endPoint))
			{
				pPainter->drawLine(m_startPoint, m_endPoint);
			}
		}
	}

	void CLine::mouseMoveEvent(QMouseEvent * pEvent)
	{
		if(m_bIsPressed)
		{
			m_endPoint = pEvent->pos();
		}
	}

	void CLine::mousePressEvent(QMouseEvent * pEvent)
	{
		m_startPoint = pEvent->pos();
		m_endPoint = pEvent->pos();
		m_bIsPressed = true;
	}

	void CLine::mouseReleaseEvent(QMouseEvent * pEvent)
	{
		m_endPoint = pEvent->pos();

    
		points.m_pStart = m_startPoint;
		points.m_pEnd = m_endPoint;
		points.m_color = m_color;
		m_pointList.push_back(points);

		m_startPoint = m_endPoint;

		m_bIsPressed = false;
	}


	////////////////////////////////////////////////////////////////Circle
	CCircle::CCircle(QColor color)
		:CDrawUnit()
		 ,m_bIsPressed(false)
		 ,m_color(color)
	{

	}

	CCircle::~CCircle()
	{

	}

	void CCircle::Render(QPainter* pPainter)
	{
		if(pPainter)
		{
			pPainter->setPen(QPen(m_color));
			for(PointList::iterator itr = m_pointList.begin(); itr != m_pointList.end(); ++itr)
			{
				LinePoints points = *itr;
				pPainter->setPen(QPen(points.m_color));
				pPainter->drawEllipse(CreateRect(points.m_pStart, points.m_pEnd));
			}

			if(m_bIsPressed && (m_startPoint != m_endPoint))
			{
				pPainter->drawEllipse(CreateRect(m_startPoint, m_endPoint));
			}
		}
	}

	void CCircle::mouseMoveEvent(QMouseEvent * pEvent)
	{
		if(m_bIsPressed)
		{
			m_endPoint = pEvent->pos();
		}
	}

	void CCircle::mousePressEvent(QMouseEvent * pEvent)
	{
		m_startPoint = pEvent->pos();
		m_endPoint = pEvent->pos();
		m_bIsPressed = true;
	}

	void CCircle::mouseReleaseEvent(QMouseEvent * pEvent)
	{
		m_endPoint = pEvent->pos();

		points.m_pStart = m_startPoint;
		points.m_pEnd = m_endPoint;
		points.m_color = m_color;

		m_pointList.push_back(points);

		m_startPoint = m_endPoint;

		m_bIsPressed = false;
	}


	////////////////////////////////////////////////////////////////Rect
	CRect::CRect(QColor color)
		:CDrawUnit()
		 ,m_bIsPressed(false)
		 ,m_color(color)
	{
	
	}

	CRect::~CRect()
	{

	}

	void CRect::Render(QPainter* pPainter)
	{
		if(pPainter)
		{	
			pPainter->setPen(QPen(m_color));
			for(PointList::iterator itr = m_pointList.begin(); itr != m_pointList.end(); ++itr)
			{
				LinePoints points = *itr;
				pPainter->setPen(QPen(points.m_color));
				pPainter->drawRect(CreateRect(points.m_pStart, points.m_pEnd));
			}

			if(m_bIsPressed && (m_startPoint != m_endPoint))
			{
				pPainter->drawRect(CreateRect(m_startPoint, m_endPoint));
			}
		}
	}

	void CRect::mouseMoveEvent(QMouseEvent * pEvent)
	{
		if(m_bIsPressed)
		{
			m_endPoint = pEvent->pos();
		}
	}

	void CRect::mousePressEvent(QMouseEvent * pEvent)
	{
		m_startPoint = pEvent->pos();
		m_endPoint = pEvent->pos();
		m_bIsPressed = true;
	}

	void CRect::mouseReleaseEvent(QMouseEvent * pEvent)
	{
		m_endPoint = pEvent->pos();

		points.m_pStart = m_startPoint;
		points.m_pEnd = m_endPoint;
		points.m_color = m_color;

		m_pointList.push_back(points);;

		m_startPoint = m_endPoint;

		m_bIsPressed = false;
	}

}
