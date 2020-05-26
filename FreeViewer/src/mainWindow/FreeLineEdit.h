/**************************************************************************************************
* @file FreeLineEdit.h
* @note 重写QlineEdit类
* @author x00038
* @data 2018-10-23
**************************************************************************************************/
#ifndef		FREE_LINE_EDIT_H
#define		FREE_LINE_EDIT_H 1

//qt提升的类 不能放入namespace 不然会找不到

#include <QLineEdit>
#include <QMouseEvent>

	class CFreeLineEdit : public QLineEdit
	{
		Q_OBJECT

	public:
		CFreeLineEdit(QWidget* pParent = 0);

		~CFreeLineEdit();

	protected:
		/**  
		  * @note 重写鼠标左键点击事件 发送clicked信号
		*/
		virtual void mousePressEvent(QMouseEvent *e);

signals:
		 void clicked();
	};


#endif	//FREE_LINE_EDIT_H