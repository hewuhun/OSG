/**************************************************************************************************
* @file FloodAnalysisWidget.h
* @note 淹没分析的结果显示界面
* @author c00005
* @data 2015-8-31
**************************************************************************************************/
#ifndef FLOOD_ANALYSIS_WIDGET_H
#define FLOOD_ANALYSIS_WIDGET_H 1

#include <ui_FloodAnalysisWidget.h>

#include <FeMeasure/FloodAnalysisMeasure.h>

#include <measure/MeasureDisplayWidget.h>

namespace FreeViewer
{
	/**
	  * @class CMFloodAnalysisWidget
	  * @brief 淹没分析的结果显示界面
	  * @author c00005
	*/
	class CMFloodAnalysisWidget : public CMeasureDisplayWidget
	{
		Q_OBJECT

	public:
		CMFloodAnalysisWidget(QWidget* parent);
		~CMFloodAnalysisWidget();

	private:
		/**  
		  * @note 派生类实现该接口类处理测量消息，输入参数有效
		*/
		virtual void HandleMeasureMsg(EMeasureAction msg, CMeasure* pMeasure);

		/**  
		  * @note 更新显示
		*/
		void UpdateDisplay();

	protected slots:
		void SlotBeginAnalyze();

		void SlotPauseAnalyze();
		
		void SlotReBeginAnalyze();

		void SlotBorderShow(bool);

	protected:
		Ui::FloodAnalysisWidget			ui;
		osg::observer_ptr<FeMeasure::CFloodAnalysisMeasure> m_opFloodAnalysisMeasure;
	};
}
#endif //FLOOD_ANALYSIS_WIDGET_H