/**************************************************************************************************
* @file FreeDockWidget.h
* @note 悬浮窗体
* @author w00040
* @data 2017-1-14
**************************************************************************************************/
#ifndef FREE_DOCK_WIDGET_H
#define FREE_DOCK_WIDGET_H

#include <QWidget>

#include "ui_FreeDockWidget.h"

namespace FreeViewer
{
	/**
	  * @class CFreeDockWidget
	  * @brief 悬浮窗体
	  * @note 悬浮窗体，实现窗体的隐藏和悬停
	  * @author w00040
	*/
	class CFreeDockWidget : public QWidget
	{
		Q_OBJECT

	public:
		/**  
		 * @brief 构造函数
		 */
		CFreeDockWidget(QString titleText, QWidget *parent = 0);

		/**  
		 * @brief 析构函数
		 */
		~CFreeDockWidget(){}

		/** 
		 * @brief 初始化数据
		 */
		void InitData();

		/**  
		  * @brief 添加子窗口
		*/
		void AddSubWidget(QWidget *pWidget);

	protected slots:
		/**
		 * @brief 悬浮窗隐藏槽函数
		 */
		void SlotHideBtnClicked(bool);

	signals:
		/**
		 * @brief 隐藏悬浮窗时发送标题栏文本信号
		 */
		void SignalSendTitleText(const QString str);

	public:
		Ui::CFreeDockWidget ui;
	};
}



#endif // FREE_DOCK_WIDGET_H
