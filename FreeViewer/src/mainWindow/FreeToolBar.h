/**************************************************************************************************
* @file FreeToolBar.h
* @note 自定义工具条，可添加功能按钮，并可循环拖动工具条改变按钮的位置
* @author c00005
* @data 2017-1-3
**************************************************************************************************/
#ifndef FREE_TOOL_WIDGET_H
#define FREE_TOOL_WIDGET_H 1

#include <QWidget>

namespace FreeViewer
{
	class CFreeToolButton;

	/**
	  * @class CFreeToolBar
	  * @brief 自定义工具条
	  * @note 自定义工具条，可添加功能按钮，并可循环拖动工具条改变按钮的位置
	  * @author c00005
	*/
    class CFreeToolBar : public QWidget
    {
        Q_OBJECT
	
    public:
		/**  
		  * @note 构造函数
		*/
        CFreeToolBar( QWidget* pParent=0 );

		/**  
		  * @note 析构函数
		*/
		~CFreeToolBar();

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
		  * @note 向工具栏中添加按钮  
		  * @param pToolBtn [in] 工具栏按钮
		*/
		void AddToolButton(CFreeToolButton* pToolBtn);

		/**  
		  * @note 删除工具栏中的按钮  
		  * @param pToolBtn [in] 工具栏按钮
		*/
		bool RemoveToolButton(CFreeToolButton* pToolBtn);

		/**  
		  * @note 添加间隔符按钮
		*/
		void AddSperator();

	protected:
		/**  
		  * @note 重新计算工具栏的总长度
		*/
		void RecalcTotalWidth();

		/**  
		  * @note 重新绘制工具栏窗口，使得工具栏中的按钮重新排布
		*/
		void RepaintWidgets();

		/**  
		  * @note 重新绘制循环拖动工具栏窗口
		  * @param currentPos [in] 当前鼠标的位置
		*/
		void RepaintLoopWidgets(QPoint currentPos);

		/**  
		  * @note 重新绘制拖动工具栏窗口
		  * @param currentPos [in] 当前鼠标的位置
		*/
		void RepaintNoLoopWidgets(QPoint currentPos);

		//用于实现鼠标拖拽，改变工具栏按钮
	protected:
		/**  
		  * @note 鼠标按下事件响应
		*/
		virtual void mousePressEvent(QMouseEvent * event);
		
		/**  
		  * @note 鼠标移动事件响应
		*/
		virtual void mouseMoveEvent(QMouseEvent * event);

		/**  
		  * @note 鼠标释放事件响应
		*/
		virtual void mouseReleaseEvent(QMouseEvent * event);

		/**  
		  * @note 重置工具栏大小事件响应
		*/
		virtual void resizeEvent(QResizeEvent * event);

		/**  
		  * @brief 事件监听
		  * @note 用于监听子按钮的鼠标按下、拖动、释放事件，主要实现工具条中拖动任意位置均可实现拖动 
		*/
		virtual bool eventFilter(QObject * object, QEvent* event);

	protected:
		///是否为循环拖拽移动
		bool							m_bLooping;

		///当前鼠标是否已经按下
		bool							m_bMousePressed;

		///工具栏按钮移动的偏移位置
		int								m_nDragDeltaX;

		///工具栏中按钮的总长度
		int								m_nTotalWidth;

		///上一次鼠标点击的位置
		QPoint							m_lastPoint;

		///用于保存工具栏中按钮的列表
		typedef	QList<QWidget*>			WidgetLists;
		WidgetLists						m_listWidgets;
    };
}
#endif 







