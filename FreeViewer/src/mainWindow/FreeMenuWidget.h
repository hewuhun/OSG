/**************************************************************************************************
* @file FreeMenuWidget.h
* @note 自定义菜单栏，实现了平台logo、菜单栏和工具栏
* @author c00005
* @data 2016-12-22
**************************************************************************************************/
#ifndef FREE_MENU_WIDGET_H
#define FREE_MENU_WIDGET_H 1

#include <QWidget>
#include <QMenuBar>

#include "ui_FreeMenuWidget.h"

namespace FreeViewer
{
	class CFreeToolBar;

	/**
	* @class CFreeMenuWidget
	* @brief 自定义菜单栏
	* @note 自定义菜单栏，实现了平台logo、菜单栏和工具栏
	* @author c00005
	*/
	class CFreeMenuWidget : public QWidget
	{
		Q_OBJECT

	public:
		/**  
		* @note 构造函数
		*/
		CFreeMenuWidget( QWidget* pParent=0 );

		/**  
		* @note 析构函数
		*/
		~CFreeMenuWidget();

	public:
		/**  
		* @brief 创建或获取菜单
		* @return QMenu* 菜单
		*/
		QMenu* GetOrCreateMenu(QString strName);

		/**  
		* @brief 获取菜单栏，可向其中添加菜单
		* @return QMenuBar* 菜单栏
		*/
		QMenuBar* GetMenuBar();

		/**  
		* @brief 获取工具栏，可向其中添加工具按钮和分隔符
		* @return CFreeToolBar* 自定义工具栏
		*/
		CFreeToolBar* GetToolBar();

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
		* @note 计算外圈边框的顶点 
		* @return QVector<QPoint> 顶点集合
		*/
		QVector<QPoint> CalculateOutterPoints();

		/**  
		* @note 计算内圈边框的顶点 
		* @return QVector<QPoint> 顶点集合
		*/
		QVector<QPoint> CalculateInnerPoints();

		/**  
		* @note 绘制事件响应
		*/
		virtual void paintEvent(QPaintEvent * event);

		virtual void resizeEvent(QResizeEvent * event);
	signals:
		/**  
		* @note 最小化信号
		*/
		void SignalMinisizeWidget();
		
		/**  
		* @note 还原信号
		*/
		void SignalMaxisizeWidget();
		
		/**  
		* @note 关闭信号
		*/
		void SignalCloseWidget();

	protected slots:
		/**  
		* @note 最小化窗口的槽函数
		*/
		void SlotMinisizeWidget();
	
		/**  
		* @note 还原窗口的槽函数
		*/
		void SlotMaxisizeWidget();
	
		/**  
		* @note 关闭窗口的槽函数
		*/
		void SlotCloseWidget();

	protected:
		///菜单栏
		QMenuBar*						m_pMenuBar;

		///工具栏
		CFreeToolBar*					m_pToolBar;

		///UI
		Ui_CFreeMenuWidget					ui;
	};

}
#endif 







