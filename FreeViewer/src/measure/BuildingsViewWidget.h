/**************************************************************************************************
* @file LineBuildingsViewWidget.h
* @note 建筑群视域分析功能设置
* @author g000034
* @data 2016-10-10
**************************************************************************************************/
#ifndef BUILDINGS_VIEW_WIDGET_H
#define BUILDINGS_VIEW_WIDGET_H 1

#include <ui_BuildingsViewWidget.h>

#include <FeMeasure/BuildingsViewMeasure.h>

#include <measure/MeasureDisplayWidget.h>

namespace FreeViewer
{
	using namespace FeMeasure;

	/**
	  * @class CMBuildingsViewWidget
	  * @brief  建筑群视域分析功能设置窗口
	  * @author g000034
	*/
	class CMBuildingsViewWidget : public CMeasureDisplayWidget
	{
		Q_OBJECT

	public:
		CMBuildingsViewWidget(QWidget* parent);

		~CMBuildingsViewWidget();

	private:
		/**  
		  * @note 派生类实现该接口类处理测量消息，输入参数有效
		*/
		virtual void HandleMeasureMsg(EMeasureAction msg, CMeasure* pMeasure);
	
	protected slots:
		void SlotPositionXChanged(double val);
		void SlotPositionYChanged(double val);
		void SlotPositionZChanged(double val);
		void SlotNearClipChanged(double val);
		void SlotFarClipChanged(double val);
		void SlotFovChanged(double val);
		void SlotFovAspectChanged(double val);
		void SlotPitchAngleChanged(double val);
		void SlotLineColorChanged();

	protected:
		Ui::BuildingsViewWidget				ui;
		osg::observer_ptr<CBuildingsViewMeasure> m_opBuildingViewMeasure;
	};
}
#endif //BUILDINGS_VIEW_WIDGET_H