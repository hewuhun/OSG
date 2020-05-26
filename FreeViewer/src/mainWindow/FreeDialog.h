/**************************************************************************************************
* @file FreeDialog.h
* @note 统一风格属性对话框
* @author w00040
* @data 2017-1-14
**************************************************************************************************/
#ifndef FREE_DIALOG_H
#define FREE_DIALOG_H

#include <mainWindow/FreeFramelessWidget.h>
#include <QLabel>
#include <QPushButton>
#include <QStyleOption>
#include <QPainter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>

namespace FreeViewer
{
	/**
	  * @class CFreeDialog
	  * @brief 统一风格属性对话框界面
	  * @note 属性对话框界面,实现统一样式的对话框界面
	  * @author w00040
	*/
	class CFreeDialog : public CFreeFramelessWidget
	{
		Q_OBJECT

	public:
		/**  
		 * @brief 构造函数
		 */
		CFreeDialog(QWidget *parent = 0);

		/**  
		 * @brief 析构函数
		 */
		~CFreeDialog(void);

		/** 
		 * @brief 初始化数据
		 */
		void InitData();

		/** 
		 * @brief 设置标题栏文本
		 */
		void SetTitleText(QString text);

		/**  
		  * @brief 添加窗体到对话框布局
		*/
		void AddWidgetToDialogLayout(QWidget *widget);

	protected:
		/**  
		* @brief 鼠标双击事件响应 
		*/
		virtual void mouseDoubleClickEvent(QMouseEvent* event);
		
		/**  

		* @brief 大小改变事件响应 
		*/
		virtual void resizeEvent(QResizeEvent* event);

	private:
		/** 
		 * @brief 设置标题栏
		 */
		void SetTitleWidget();

	protected slots:
		/** 
		 * @brief 关闭按钮槽函数
		 */
		void SlotCloseBtnClicked();

	protected:
		///对话框标题栏
		QWidget*					m_pTitleWidget;

		///对话框整体布局
		QVBoxLayout*					m_pVLayout;

		///标题栏图标
		QLabel*						m_pTitleIcon;

		///标题栏文本
		QLabel*						m_pTitleText;

		///关闭按钮
		QPushButton*				m_pCloseBtn;
	};
}

#endif
