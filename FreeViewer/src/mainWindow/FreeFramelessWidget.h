/**************************************************************************************************
* @file FreeFramelessWidget.h
* @note 无边框窗体基类，所有实现无边框窗体并且可拖动改变大小均可从本类继承
* @author c00005
* @data 2017-1-12
**************************************************************************************************/
#ifndef FREE_FRAMELESS_WIDGET_H
#define FREE_FRAMELESS_WIDGET_H 1

#include <QWidget>

namespace FreeViewer
{
	/**
	  * @class CFreeFramelessWidget
	  * @brief 无边框窗体基类
	  * @note 无边框窗体基类，所有实现无边框窗体并且可拖动改变大小均可从本类继承
	  * @author c00005
	*/
	class CFreeFramelessWidget : public QWidget
	{
		Q_OBJECT

	public:
		/**  
		* @note 构造函数
		*/
        CFreeFramelessWidget(QWidget *parent = 0);

		/**  
		* @note 析构函数
		*/
		~CFreeFramelessWidget();

	protected:
		/**  
		* @note 初始化窗口 
		*/
		void InitWidget();

	public:
		/**  
		  * @note 设置是否可缩放窗口  
		  * @param bState [in] 是否可缩放窗口
		*/
		void SetResizeWidget(bool bState);

		/**  
		  * @note 设置可点击拖拽移动的区域  
		  * @param sizeArea [in] 区域
		*/
		void SetMovableArea(QRect rectArea);
		
		/**  
		  * @note 设置可点双击改变窗口大小的区域  
		  * @param sizeArea [in] 区域
		*/
		void SetDoubleClickedArea(QRect rectArea);

		/**  
		* @note 最大化显示 
		*/
		void ShowDialogMaxisize();

		/**  
		* @note 最小化显示 
		*/
		void ShowDialogMinisize();

		/**  
		* @note 正常显示 
		*/
		void ShowDialogNormal();

	protected:
		/**  
		* @note 检查鼠标是否在窗体的边界处 
		*/
		void CheckEdge();
		
		/**  
		* @note 判断一个点是否在一个区域内
		* @param sizeArea [in] 区域
		* @param sizeArea [in] 点
		* @return bool 点否是在此区域内
		*/
		bool PointInArea(const QRect& rectArea, const QPoint& point);

	public slots:
		/**  
		* @note 关闭窗体的槽函数 
		*/
		virtual void SlotCloseWidget();

		/**  
		* @note 最小化窗体的槽函数 
		*/
		virtual void SlotMinisizeWidget();

		/**  
		* @note 最大化窗体的槽函数 
		*/
		virtual void SlotMaxisizeWidget();

	protected:
		/**  
		* @note 鼠标按下事件响应 
		*/
		virtual void mousePressEvent(QMouseEvent *event);

		/**  
		* @note 鼠标释放事件响应 
		*/
		virtual void mouseReleaseEvent(QMouseEvent *event);

		/**  
		* @note 鼠标移动事件响应 
		*/
		virtual void mouseMoveEvent(QMouseEvent *event);

		/**  
		* @note 鼠标双击事件响应 
		*/
		virtual void mouseDoubleClickEvent(QMouseEvent* event);

	protected:
		///窗口是否还原显示或者最大化显示
		bool												m_bShowNormal;

		///窗口在鼠标按下一刻的位置
		QPoint												m_pointLastMouse;

		///鼠标是否按下
		bool												m_bMousePress;

		///窗口是否可缩放
		bool												m_bResizeWidget;

		///鼠标检测的边缘距离
		int													m_nEdgeMargin;     
	
		///拖拽移动窗口的区域
		QRect												m_rectMovableArea;

		///双击放大缩小窗口的区域
		QRect												m_rectDoubleClickedArea;

		//更改窗口大小的方向
		typedef enum
		{
			NO_DIR						= 0x00,
			TOP							= 0x01,
			BOTTOM						= 0x02,
			LEFT						= 0x04,
			RIGHT						= 0x08,
			TOP_LEFT					= 0x01 | 0x04,
			TOP_RIGHT					= 0x01 | 0x08,
			BOTTOM_LEFT					= 0x02 | 0x04,
			BOTTOM_RIGHT				= 0x02 | 0x08
		} EMouseDir;
		EMouseDir											m_nMouseDir;
	};

}

#endif 
