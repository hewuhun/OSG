/**************************************************************************************************
* @file FillCutAnalysisWidget.h
* @note 填挖方的结果显示界面
* @author c00005
* @data 2015-8-25
**************************************************************************************************/
#ifndef FIll_Cut_ANALYSIS_WIDGET_H
#define FIll_Cut_ANALYSIS_WIDGET_H 1

#include <ui_FillCutAnalysisWidget.h>

#include <FeMeasure/FillCutAnalysisMeasure.h>

#include <measure/MeasureDisplayWidget.h>

namespace FreeViewer
{
	/**
	  * @class CMFillCutAnalysisWidget
	  * @brief 填挖方分析的结果显示界面
	  * @note 填挖方分析的结果显示界面
	  * @author c00005
	*/
	class CMFillCutAnalysisWidget : public CMeasureDisplayWidget
	{
		Q_OBJECT

	public:
		CMFillCutAnalysisWidget(QWidget* parent);

		~CMFillCutAnalysisWidget();

	private:
		/**  
		  * @note 派生类实现该接口类处理测量消息，输入参数有效
		*/
		virtual void HandleMeasureMsg(EMeasureAction msg, CMeasure* pMeasure);

	protected slots:
		/**  
		  * @note 开始分析
		*/
		void SlotBeginAnalyze();

	protected:
		Ui::FillCutAnalysisWidget				ui;
		osg::observer_ptr<FeMeasure::CFillCutAnalysisMeasure> m_opFillCutAnalysisMeasure;
	};
}
#endif //FIll_Cut_ANALYSIS_WIDGET_H