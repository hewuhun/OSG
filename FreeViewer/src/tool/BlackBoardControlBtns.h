/**************************************************************************************************
* @file BlackBoardControlBtns.h
* @note 黑板控制按钮
* @author w00040
* @data 2017-3-13
**************************************************************************************************/
#ifndef BLACK_BOARD_CONTROL_BTNS_H
#define BLACK_BOARD_CONTROL_BTNS_H

#include <tool/BlackBoard.h>

#include <QWidget>
#include "ui_BlackBoardControlBtns.h"

namespace FreeViewer
{
	/**
	  * @class CBlackBoardControlBtns
	  * @brief 黑板控制按钮类
	  * @note 实现黑板控制按钮操作
	  * @author w00040
	*/
	class CBlackBoardControlBtns : public QWidget
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CBlackBoardControlBtns(QWidget *parent = 0);

		/**  
		  * @brief 析构函数
		*/
		~CBlackBoardControlBtns();

		/**  
		  * @brief 初始化界面
		*/
		void InitWidget();

		/**  
		  * @brief 设置画布
		*/
		void SetBlackBoardWidget(CBlackBoard* pBlackBoard);

	private slots:
		/**  
			* @brief 擦除槽函数
		*/
		void SlotOnClearBtnClicked(bool checked);

		/**  
			* @brief 退出槽函数
		*/
		void SlotOnExitBtnClicked(bool checked);

		/**  
			* @brief 鼠标绘制槽函数
		*/
		void SlotOnMousePathBtnClicked(bool checked);

		/**  
			* @brief 线绘制槽函数
		*/
		void SlotOnLineStripBtnClicked(bool checked);

		/**  
			* @brief 圆绘制槽函数
		*/
		void SlotOnCircleBtnClicked(bool checked);

		/**  
			* @brief 矩形绘制槽函数
		*/
		void SlotOnRectBtnClicked(bool checked);

		/**  
			* @brief 颜色设置槽函数
		*/
		void SlotOnColorBtnClicked(bool checked);

	private:
		Ui::BlackBoardControlBtns_Widget ui;

		///画布
		CCanvas*		m_pCanvas;

		///判断当前正在绘制的图形
		int				m_state;

		///黑板
		CBlackBoard*	m_pBlackBoardWidget;
	};
}

#endif //BLACK_BOARD_CONTROL_BTNS_H
