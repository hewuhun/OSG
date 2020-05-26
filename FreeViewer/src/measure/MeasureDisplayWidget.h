/**************************************************************************************************
* @file MeasureDisplayWidget.h
* @note 测量结果显示窗口
* @author g00034
* @data 2017-1-19
**************************************************************************************************/

#ifndef MEASURE_DISPLAY_WIDGET_H
#define MEASURE_DISPLAY_WIDGET_H 1

#include <mainWindow/FreeDialog.h>
#include <FeMeasure/Measure.h>


namespace FreeViewer
{
	using namespace FeMeasure;

	/**
	  * @class CMeasureDisplayWidget
	  * @brief 测量结果显示对话框基类，所有测量结果对话框从此类派生
	  * @note  其中实现了通用操作，派生类实现 HandleMeasureMsg 接口来处理测量消息
	  * @author g00034
	*/
	class CMeasureDisplayWidget : public CFreeDialog, public CMeasureCallback
	{
	public:
		virtual ~CMeasureDisplayWidget(){}

		/**  
		  * @note 接收测量消息，处理结果。由 SDK 调用  
		  * @param msg [in] 消息类型
		  * @param pMeasure [in] 测量对象
		  * @return 无返回值
		*/
		void SendMsg(EMeasureAction msg, CMeasure* pMeasure);

		/**  
		  * @note 处理关闭对话框的系统事件，进行特殊操作
		*/
		void closeEvent (QCloseEvent * e);


	protected:
		CMeasureDisplayWidget(QWidget* parent) : CFreeDialog(parent){}

	private:
		/**  
		  * @note 派生类实现该接口类处理测量消息，输入参数有效
		*/
		virtual void HandleMeasureMsg(EMeasureAction msg, CMeasure* pMeasure) = 0;

	private:
		/// 当前处理的测量对象
		osg::observer_ptr<CMeasure>     m_opMeasure;
	};



	/// 工具宏. 从 CMeasureDisplayWidget 派生的子类可以通过该宏装载 ui
	#define MEASURE_RESULT_DLG_SETUP_UI(ui) {    \
		QWidget* widget = new QWidget(this);     \
		ui.setupUi(widget);                      \
		AddWidgetToDialogLayout(widget);         \
		widget->setMinimumSize(widget->size());  \
		}                                                               

}


#endif //MEASURE_DISPLAY_WIDGET_H


