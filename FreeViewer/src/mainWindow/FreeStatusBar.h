/**************************************************************************************************
* @file FreeStatusBar.h
* @note 自定义状态栏，用于显示提示信息以及视点和相机的信息
* @author c00005
* @data 2017-1-12
**************************************************************************************************/
#ifndef FREE_STATUS_BAR_H
#define FREE_STATUS_BAR_H 1

#include <QWidget>

#include <QLabel>
#include <QHBoxLayout>

namespace FreeViewer
{
	/**
	  * @class CFreeStatusBar
	  * @brief 自定义状态栏
	  * @note 自定义状态栏，用于显示提示信息以及视点和相机的信息
	  * @author c00005
	*/
    class CFreeStatusBar : public QWidget
    {
        Q_OBJECT
	
    public:
		/**  
		  * @note 构造函数
		*/
        CFreeStatusBar( QWidget* pParent=0 );

		/**  
		  * @note 析构函数
		*/
		~CFreeStatusBar();

	public:
		/**  
		  * @note 设置视点位置信息以及相机信息的字符串
		*/
		void SetViewText(QString strLon, QString strLat, QString strHei, QString strCamHei);

		/**  
		  * @note 设置提示或者警告信息
		*/
		void SetMessageText(QString strText);
		
	protected:
		/**  
		  * @note 初始化窗口 
		*/
		void InitWidget();

		/**  
		  * @note 初始化窗口上下文
		*/
		void InitContext();

	protected:
		/**  
		  * @note 绘制事件响应，用于绘制上边线
		*/
		virtual void paintEvent(QPaintEvent * event);

	protected:
		///窗体的整体布局
		QHBoxLayout*	m_pLayout;

		///提示信息显示框
		QLabel*			m_pMessLabel;

		///相机信息显示框
		QLabel*			m_pViewLable;
    };
}
#endif 







