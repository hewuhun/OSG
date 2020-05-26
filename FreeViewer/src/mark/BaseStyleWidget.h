
#ifndef BASE_TAB_WIDGET_H
#define BASE_TAB_WIDGET_H
#include <QWidget>

namespace FreeViewer
{
	/**
	* @class 标签页基类
	* @brief 留给子类重写确定取消
	* @author z00019
	*/
	class CBaseStyleWidget : public QWidget
	{
		Q_OBJECT
	public:
		
		CBaseStyleWidget(QWidget *parent = 0);
		~CBaseStyleWidget();

	public:
		/**  
		  * @brief 确定修改属性信息
		  * @return bool
		*/
		virtual bool Accept();

		/**  
		  * @brief 拒绝修改属性信息
		  * @return bool
		*/
		virtual bool Reject();

	protected slots:
		/**  
		  * @note SDK 事件处理槽函数
		*/
		virtual void SlotSDKEventHappend(unsigned int){}
	};

}
#endif // BASE_TAB_WIDGET_H

