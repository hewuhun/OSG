/**************************************************************************************************
* @file DrawUnit.h
* @note 绘制单元
* @author w00040
* @data 2017-2-27
**************************************************************************************************/
#ifndef CDRAW_UNIT_H
#define CDRAW_UNIT_H

#include <QWidget>
#include <QPainter>
#include <QPoint>
#include <QVector>
#include <QMouseEvent>

namespace FreeViewer
{
	/**
	  * @class CDrawUnit
	  * @brief 绘制单元基类
	  * @note 实现绘制单元的基类
	  * @author w00040
	*/
	class CDrawUnit
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CDrawUnit(){};

		/**  
		  * @brief 析构函数
		*/
		virtual ~CDrawUnit(){};

		virtual void Render(QPainter* pPainter) = 0;

		/**  
		  * @brief 鼠标移动事件
		*/
		virtual void mouseMoveEvent(QMouseEvent * pEvent) = 0;

		/**  
		  * @brief 鼠标点击事件
		*/
		virtual void mousePressEvent(QMouseEvent * pEvent) = 0;

		/**  
		  * @brief 鼠标释放事件
		*/
		virtual void mouseReleaseEvent(QMouseEvent * pEvent) = 0;
	};


	/**
	  * @class CMousePath
	  * @brief 鼠标路径
	  * @note 绘制鼠标路径
	  * @author w00040
	*/
	class CMousePath : public CDrawUnit
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CMousePath(QColor color);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CMousePath();

		virtual void Render(QPainter* pPainter);

		/**  
		  * @brief 鼠标移动事件
		*/
		virtual void mouseMoveEvent(QMouseEvent * pEvent);

		/**  
		  * @brief 鼠标点击事件
		*/
		virtual void mousePressEvent(QMouseEvent * pEvent);

		/**  
		  * @brief 鼠标释放事件
		*/
		virtual void mouseReleaseEvent(QMouseEvent * pEvent);

	private:
		///鼠标开始点
		QPoint      m_startPoint;

		///鼠标结束点
		QPoint      m_endPoint;

		///颜色
		QColor		m_color;

		struct LinePoints
		{
			QPoint    m_pStart;
			QPoint    m_pEnd;
			QColor	  m_color;
		}points;

		typedef QVector<LinePoints>  PointList;
		PointList   m_pointList;

		bool        m_bIsPressed;
	};


	/**
	  * @class CLine
	  * @brief 线
	  * @note 绘制线
	  * @author w00040
	*/
	class CLine : public CDrawUnit
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CLine(QColor color);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CLine();

		virtual void Render(QPainter* pPainter);

		/**  
		  * @brief 鼠标移动事件
		*/
		virtual void mouseMoveEvent(QMouseEvent * pEvent);

		/**  
		  * @brief 鼠标点击事件
		*/
		virtual void mousePressEvent(QMouseEvent * pEvent);

		/**  
		  * @brief 鼠标释放事件
		*/
		virtual void mouseReleaseEvent(QMouseEvent * pEvent);

	private:
		///鼠标开始点
		QPoint      m_startPoint;

		///鼠标结束点
		QPoint      m_endPoint;

		///颜色
		QColor      m_color;

		struct LinePoints
		{
			QPoint    m_pStart;
			QPoint    m_pEnd;
			QColor	  m_color;
		}points;

		typedef QVector<LinePoints>  PointList;
		PointList   m_pointList;

		bool        m_bIsPressed;
	};

	/**
	  * @class CCircle
	  * @brief 圆
	  * @note 绘制圆
	  * @author w00040
	*/
	class CCircle : public CDrawUnit
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CCircle(QColor color);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CCircle();

		virtual void Render(QPainter* pPainter);

		/**  
		  * @brief 鼠标移动事件
		*/
		virtual void mouseMoveEvent(QMouseEvent * pEvent);

		/**  
		  * @brief 鼠标点击事件
		*/
		virtual void mousePressEvent(QMouseEvent * pEvent);

		/**  
		  * @brief 鼠标释放事件
		*/
		virtual void mouseReleaseEvent(QMouseEvent * pEvent);

	private:
		///鼠标开始点
		QPoint      m_startPoint;

		///鼠标结束点
		QPoint      m_endPoint;

		///颜色
		QColor		m_color;

		struct LinePoints
		{
			QPoint    m_pStart;
			QPoint    m_pEnd;
			QColor	  m_color;
		}points;

		typedef QVector<LinePoints>  PointList;
		PointList   m_pointList;

		bool        m_bIsPressed;
	};


	/**
	  * @class CRect
	  * @brief 矩形
	  * @note 绘制矩形
	  * @author w00040
	*/
	class CRect : public CDrawUnit
	{
	public:
		/**  
		  * @brief 构造函数
		*/
		CRect(QColor color);

		/**  
		  * @brief 析构函数
		*/
		virtual ~CRect();

		virtual void Render(QPainter* pPainter);

		/**  
		  * @brief 鼠标移动事件
		*/
		virtual void mouseMoveEvent(QMouseEvent * pEvent);

		/**  
		  * @brief 鼠标点击事件
		*/
		virtual void mousePressEvent(QMouseEvent * pEvent);

		/**  
		  * @brief 鼠标释放事件
		*/
		virtual void mouseReleaseEvent(QMouseEvent * pEvent);

	private:
		QPoint      m_startPoint;
		QPoint      m_endPoint;
		QColor		m_color;

		struct LinePoints
		{
			QPoint    m_pStart;
			QPoint    m_pEnd;
			QColor	  m_color;
		}points;

		typedef QVector<LinePoints>  PointList;
		PointList   m_pointList;

		bool        m_bIsPressed;
	};

}

#endif // CDRAWUNIT_H
