/**************************************************************************************************
* @file BootScreenFrame.h
* @note 启动界面
* @author w00040
* @data 2017-2-23
**************************************************************************************************/
#ifndef BOOT_SCREEN_FRAME_H
#define BOOT_SCREEN_FRAME_H

#include <QWidget>
#include <QSplashScreen>
#include <QGraphicsEffect>
#include <QTimer>
#include <QPainter>
#include <QMovie>

#include <mainWindow/BootScreenRoundBar.h>
#include <mainWindow/BootScreenBallPath.h>

#include "ui_BootScreenFrame.h"

#include <QTime>

namespace FreeViewer
{
	/**
	  * @class CBootScreenFrame
	  * @brief 启动界面
	  * @note 设置启动界面
	  * @author w00040
	*/
	class CBootScreenFrame : public QSplashScreen
	{
		Q_OBJECT

	public:
		CBootScreenFrame(QWidget *parent = 0);
		~CBootScreenFrame();

		/**
		 * @brief 初始化启动界面
		 */
		void InitBootSceen();

		/**
		 * @brief 设置圆形进度条
		 */
		void SetRoundProgress(int start_value, int end_value);

		/**
		 * @brief 设置线程圆形进度条,线程结束state为true
		 */
		void SetRoundProgress(int start_value, int end_value, bool state);

		/**
		 * @brief 显示正在加载到哪个模块
		 */
		void SetLoadText(QString str1, QString str2, QString str3);

		/**
		 * @brief 加载圆形进度条
		 */
		void LoadRonudBar();

		/**
		 * @brief 设置透明度，实现图片淡入淡出 true 是淡入
		 */
		void SetWidgetOpacity(bool bIN);

		/**  
		  * @brief 主进程休眠
		*/
		void sleep(unsigned int msec);

	protected slots:
		/**
		  * @brief 设置动画widget图片
		 */
		void SetFlashImage();

	private:
		Ui::CBootScreenFrame ui;

		QGraphicsOpacityEffect*				m_effect;
		QTimer*								m_timer; //计时器
		int									m_nTimerCount; //计时器计数

		CBootScreenRoundBar*				m_pRoundBar; //圆形进度条

		int									m_nStartVal; //进度条开始值
		int									m_nEndVal; //进度条结束值

		CBootScreenBallPath*				m_pBallPath; //小球运动路径
	};
}

#endif //BOOT_SCREEN_FRAME_H
