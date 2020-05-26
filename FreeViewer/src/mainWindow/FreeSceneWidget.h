/**************************************************************************************************
* @file FreeSceneWidget.h
* @note 管理场景渲染的窗口，可以向其中添加二维渲染窗口和三维渲染窗口
* @author c00005
* @data 2017-1-12
**************************************************************************************************/
#ifndef FREE_SCENE_WIDGET_H
#define FREE_SCENE_WIDGET_H 1

#include <mainWindow/FreeFramelessWidget.h>

#include <QSplitter>
#include <QHBoxLayout>

namespace FreeViewer
{
	/**
	* @class CFreeSceneWidget
	* @brief 管理场景渲染的窗口
	* @note 管理场景渲染的窗口，可以向其中添加二维渲染窗口和三维渲染窗口
	* @author c00005
	*/
	class CFreeSceneWidget : public QSplitter 
	{
		Q_OBJECT;

	public:
		/**  
		* @note 构造函数
		*/
		CFreeSceneWidget(QWidget* parent);

		/**  
		* @note 析构函数
		*/
		~CFreeSceneWidget();

	protected:
		/**  
		* @note 初始化窗口 
		*/
		void InitWidget();

		/**  
		* @note 初始化窗口上下文 
		*/
		void InitContext();

	public:
		/**  
		* @note 添加窗口  
		* @param pWidget [in] 窗口
		*/
		void AddSubWidget(QWidget* pWidget);

		/**  
		* @note 移除窗口，将窗口指针释放，慎用
		* @param pWidget [in] 窗口
		*/
		void RemoveSubWidget(QWidget* pWidget);

		/**  
		* @note 设置窗口显隐
		* @param pWidget [in] 窗口
		* @param bVisible [in] 窗口是否可见
		*/
		void SetWidgetVisible(QWidget* pWidget, bool bVisible);

	protected:
		/**  
		  * @note 获取设置窗口的父窗口  
		  * @param pWidget [in] 当前显示窗口
		  * @return QWidget* 显示窗口的父窗口
		*/
		QWidget* GetParentWidget(QWidget* pWidget);

	protected:
		/**  
		* @note 绘制事件响应，用于绘制上边线
		*/
		virtual void paintEvent(QPaintEvent * event);

		/**  
		* @note 重置大小事件响应，用于动态改变两个窗口的大小
		*/
		virtual void resizeEvent(QResizeEvent * event);
	};
}


#endif // SCENE_VIEWER_WIDGET_H
