/**************************************************************************************************
* @file BlackBoard.h
* @note 黑板
* @author w00040
* @data 2017-2-27
**************************************************************************************************/
#ifndef BLACK_BOARD_H
#define BLACK_BOARD_H

#include <QWidget>
#include <QDialog>

#include <tool/DrawUnit.h>

#include "ui_BlackBoard.h"

namespace FreeViewer
{
	/**
	  * @class CCanvas
	  * @brief 画布
	  * @note 实现画布上的事件
	  * @author w00040
	*/
	class CCanvas : public QWidget
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		explicit CCanvas(QWidget *parent = 0);

		/**  
		  * @brief 析构函数
		*/
		~CCanvas();

		/**  
		  * @brief 清除
		*/
		void ClearCanvas();

		void ActiveDrawUnit(CDrawUnit* pDrawUnit);

	public:
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

		/**  
		  * @brief 重绘事件
		*/
		virtual void paintEvent(QPaintEvent *pEvent);

	public:
		///颜色
		QColor				m_color;

	private:
		CDrawUnit*   m_pActiveDrawUnit;

		typedef QVector<CDrawUnit*>  DrawUnits;
		DrawUnits    m_drawUnits;
	};

	/**
	  * @class CBlackBoard
	  * @brief 黑板
	  * @note 实现黑板上的操作
	  * @author w00040
	*/
	class CBlackBoard : public QDialog
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		explicit CBlackBoard(QWidget *parent = 0);

		/**  
		  * @brief 析构函数
		*/
		~CBlackBoard();

	public:
		/**  
		  * @brief 获取画布
		*/
		CCanvas* GetCanvas();

	signals:
		/**  
		  * @brief 当按下ESC按键时，代表黑板功能关闭，通知按钮
		*/
		void SignalBlackBoardDone(bool);

	protected:
		/**  
		  * @brief 键盘按下事件,处理Esc键盘事件
		*/
		virtual void keyPressEvent(QKeyEvent *event);

	private:
		Ui::BlackBoard  ui;

		///画布
		CCanvas*					m_pCanvas;
	};

}

#endif // BLACK_BOARD_H
