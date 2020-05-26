#include <QPalette>
#include <QColorDialog>
#include <tool/BlackBoard.h>

#include "ui_BlackBoard.h"
#include <iostream>

namespace FreeViewer
{
	CCanvas::CCanvas(QWidget *parent) 
		:QWidget(parent)
		,m_pActiveDrawUnit(NULL)
		,m_color(Qt::yellow)
	{

	}

	CCanvas::~CCanvas()
	{

	}

	// 设置画笔
	void CCanvas::paintEvent(QPaintEvent *pEvent)
	{
		QPainter painter(this);
		QPen pen;
		pen.setColor(m_color);
		pen.setWidth(3);
		painter.setPen(pen);

		for(DrawUnits::iterator itr = m_drawUnits.begin(); itr != m_drawUnits.end(); ++itr)
		{
			if(*itr)
			{				
				(*itr)->Render(&painter);
			}
		}

		update();
	}

	//鼠标移动
	void CCanvas::mouseMoveEvent(QMouseEvent * pEvent)
	{
		if(m_pActiveDrawUnit)
		{
			m_pActiveDrawUnit->mouseMoveEvent(pEvent);
		}
	}

	//鼠标点击
	void CCanvas::mousePressEvent(QMouseEvent * pEvent)
	{
		if(m_pActiveDrawUnit)
		{
			m_pActiveDrawUnit->mousePressEvent(pEvent);
		}
	}

	//鼠标点击释放
	void CCanvas::mouseReleaseEvent(QMouseEvent * pEvent)
	{
		if(m_pActiveDrawUnit)
		{
			m_pActiveDrawUnit->mouseReleaseEvent(pEvent);
		}
	}

	void CCanvas::ClearCanvas()
	{
		m_pActiveDrawUnit = NULL;
		DrawUnits::iterator itr = m_drawUnits.begin();
		while(itr != m_drawUnits.end())
		{
			delete (*itr);
			itr = m_drawUnits.erase(itr);
		}

		update();
	}

	void CCanvas::ActiveDrawUnit(CDrawUnit* pDrawUnit)
	{
		if(pDrawUnit)
		{
			m_pActiveDrawUnit = pDrawUnit;
			m_drawUnits.push_back(pDrawUnit);
		}
	}

	//***********************************************************************

	CBlackBoard::CBlackBoard(QWidget *parent)
		:QDialog(parent)
		,m_pCanvas(NULL)
	{
		ui.setupUi(this);

		if(parent)
		{
			setGeometry(parent->rect());
		}

		m_pCanvas = new CCanvas();
		ui.hLayout_control->addWidget(m_pCanvas);
	}

	CCanvas* CBlackBoard::GetCanvas()
	{
		return m_pCanvas;
	}

	CBlackBoard::~CBlackBoard()
	{
		if (m_pCanvas)
		{
			delete m_pCanvas;
			m_pCanvas = NULL;
		}
	}

	void CBlackBoard::keyPressEvent(QKeyEvent *event)
	{
		switch(event->key())
		{
			//进行界面退出，重写Esc键，否则重写reject()方法
		case Qt::Key_Escape:
			emit SignalBlackBoardDone(true);
			break;
		default:
			QDialog::keyPressEvent(event);
		}
	}

}