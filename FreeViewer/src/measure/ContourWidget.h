/**************************************************************************************************
* @file ContourWidget.h
* @note 等高线测量结果显示
* @author x00028
* @data 2018-10-11
**************************************************************************************************/
#ifndef CONTOUR_MEASURE_WIDGET_H
#define CONTOUR_MEASURE_WIDGET_H

#include <ui_ContourWidget.h>

#include <FeMeasure/ContourMeasure.h>

#include <measure/MeasureDisplayWidget.h>

namespace FreeViewer
{
	/**
	  * @class CContourWidget
	  * @brief 等高线测量的结果显示界面
	  * @note 等高线测量的结果显示界面
	  * @author x00028
	*/
	class CContourWidget : public CMeasureDisplayWidget
	{
		Q_OBJECT

	public:
		CContourWidget(QWidget* parent);

		~CContourWidget();

	private:
		/**  
		  * @note 派生类实现该接口类处理测量消息，输入参数有效
		*/
		virtual void HandleMeasureMsg(EMeasureAction msg, CMeasure* pMeasure);

	protected slots:
		/**  
		  * @note 开始测量
		*/
		void SlotBeginMeasure();

	protected:
		Ui::ContourWidget				ui;
		osg::observer_ptr<FeMeasure::CContourMeasure>			m_opContourMeasure;
	};
}
#endif //CONTOUR_MEASURE_WIDGET_H