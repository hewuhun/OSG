/**************************************************************************************************
* @file CustomPushButton.h
* @note 自定义按钮控件
* @author w00040
* @data 2017-3-7
**************************************************************************************************/
#ifndef CUSTOM_PUSH_BUTTON_H
#define CUSTOM_PUSH_BUTTON_H

#include <QPushButton>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QEvent>

namespace FreeViewer
{
	/**
	  * @class CCustomPushButton
	  * @brief 自定义按钮控件
	  * @note 自定义按钮控件,实现字体旋转
	  * @author w00040
	*/
	class CCustomPushButton : public QPushButton
	{
		Q_OBJECT

	public:
		/**  
		  * @brief 构造函数
		*/
		CCustomPushButton(const QString &text, QWidget *parent=0);

		/**  
		  * @brief 析构函数
		*/
		~CCustomPushButton(void);

		enum EBtnStatus
		{
			NORMAL,
			ENTER
		}; 

		/**  
		  * @brief 动态计算文本数
		*/
		void ComputeBtnTextNum();

	protected:
		/**  
		  * @brief 重绘事件，实现字体旋转
		*/
		virtual void paintEvent(QPaintEvent *event);

		/**  
		  * @brief 鼠标点击事件
		*/
		virtual void mousePressEvent(QMouseEvent *e);

		/**  
		  * @brief 鼠标进入事件
		*/
		virtual void enterEvent(QEvent *event);

		/**  
		  * @brief 鼠标离开事件
		*/
		virtual void leaveEvent(QEvent *event);

	signals:
		/**  
		  * @brief 鼠标点击信号
		*/
		void SignalBtnClicked(QString);

	protected:
		///按钮文本信息
		QString						m_text;

		///文本宽度
		int							m_nTextW;

		///文本高度
		int							m_nTextH;

		///鼠标状态
		EBtnStatus					m_BtnStatus;
	};
}

#endif