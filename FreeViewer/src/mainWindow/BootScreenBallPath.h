/**************************************************************************************************
* @file BootScreenBallPath.h
* @note 小球沿折线运动
* @author w00040
* @data 2017-1-14
**************************************************************************************************/
#ifndef BOOT_SCREEN_BALL_PATH_H
#define BOOT_SCREEN_BALL_PATH_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QVector>
#include <QTimer>

namespace FreeViewer
{
	/**
	  * @class CBootScreenBallPath
	  * @brief 小球沿折线运动
	  * @note 实现启动界面，小球沿折线运动
	  * @author w00040
	*/
	class CBootScreenBallPath : public QWidget
	{
		Q_OBJECT
	public:
		/**  
		 * @brief 构造函数
		 */
		CBootScreenBallPath(QWidget *parent = 0);

		/**  
		 * @brief 析构函数
		 */
		~CBootScreenBallPath(void);

		/**
		 * @brief 计算路径关键点
		 */
		int ComputePathPoint();

		/**
		 * @brief 绘制路径
		 */
		int DrawPath();

	protected:
		/**  
		 * @brief 重绘事件
		 */
		void paintEvent(QPaintEvent *event);

	private:
		// 路径关键点
		QVector<QPointF>	m_vecPathPoints;
	};
}

#endif