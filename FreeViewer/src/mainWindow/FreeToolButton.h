/**************************************************************************************************
* @file FreeToolButton.h
* @note 四态工具按钮，可以设置四种图标，包括正常、划过、按下、失效按钮
* @author c00005
* @data 2017-1-3
**************************************************************************************************/
#ifndef FRRE_TOOL_BUTTON_H
#define FRRE_TOOL_BUTTON_H 1

#include <QMenu>
#include <QToolButton>

namespace FreeViewer
{
	/**
	  * @class CFreeToolButton
	  * @brief 四态工具按钮
	  * @note 四态工具按钮，可以设置四种图标，包括正常、划过、按下、失效按钮
	  * @author c00005
	*/
    class CFreeToolButton:public QToolButton
    {	
		Q_OBJECT

    public:
		/**  
		  * @brief 四态工具按钮
		  * @note 四态工具按钮，可以设置四种图标，包括正常、划过、按下、失效按钮  
		  * @param strName [in] 按钮的名称
		  * @param strNormalIcon [in] 正常按钮图标
		  * @param strHoverIcon [in] 划过按钮图标
		  * @param strPressIcon [in] 按下按钮图标
		  * @param strDisableIcon [in] 失效按钮图标
		  * @param parent [in] 父窗口
		  * @return 返回值
		*/
        CFreeToolButton(
			QString strName,
			QString strNormalIcon, 
			QString strHoverIcon,
			QString strPressIcon,
			QString strDisableIcon,
			QWidget* parent);

		/**  
		  * @brief 析构函数
		*/
        ~CFreeToolButton();

	public:
		/**  
		  * @brief 为按钮添加菜单
		  * @note 为按钮添加菜单，并设置子菜单名称以及图标
		  * @param strName [in] 按钮的名称
		  * @param strNormalIcon [in] 正常按钮图标
		  * @param strHoverIcon [in] 划过按钮图标
		  * @param strPressIcon [in] 按下按钮图标
		  * @param strDisableIcon [in] 失效按钮图标
		  * @return CFreeToolButton* 工具按钮，用户可连接自己的槽函数
		*/
		CFreeToolButton* AddMenuAction(
			QString strName,
			QString strNormalIcon, 
			QString strHoverIcon,
			QString strPressIcon,
			QString strDisableIcon);

		/**  
		  * @brief 正常状态
		*/
		void Normal();

		/**  
		  * @brief 划过状态
		*/
		void Hover();

		/**  
		  * @brief 按下状态
		*/
		void Pressed();

		/**  
		  * @brief 失效状态
		*/
		void Disable();

	signals:
		void SignalClicked(bool);

	public slots:
		/**  
		  * @brief 选中状态
		*/
		void SlotChecked(bool);

		//用于控制按钮的四种状态
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
		  * @note 鼠标进入事件响应
		*/
		virtual void enterEvent(QEvent * event);

		/**  
		  * @note 鼠标离开事件响应
		*/
		virtual void leaveEvent(QEvent * event);

	protected:
		///工具按钮的菜单
		QMenu*			m_pMenu;

		///设置的正常状态按钮
		QString			m_strNormalIcon;

		///设置的划过状态按钮
		QString			m_strHoverIcon;

		///设置的按下状态按钮
		QString			m_strPressedIcon;

		///设置的失效状态按钮
		QString			m_strDisableIcon;

		///用于记录鼠标按下后是否又进行移动，如果移动，则释放鼠标时不触发clicked信号
		bool			m_bMouseMove;
    };
}
#endif 







