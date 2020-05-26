#include <measure/FloodAnalysisWidget.h>

#include <mainWindow/FreeUtil.h>
using namespace FreeViewer;

namespace FreeViewer
{
	CMFloodAnalysisWidget::CMFloodAnalysisWidget( QWidget* parent )
		: CMeasureDisplayWidget(parent)
		, m_opFloodAnalysisMeasure(NULL)
	{
		MEASURE_RESULT_DLG_SETUP_UI(ui);
		
		connect(ui.pushButton_floodBegin, SIGNAL(pressed()), this, SLOT(SlotBeginAnalyze()));
		connect(ui.pushButton_floodPause, SIGNAL(pressed()), this, SLOT(SlotPauseAnalyze()));
		connect(ui.pushButton_floodReBegin, SIGNAL(pressed()), this, SLOT(SlotReBeginAnalyze()));
		connect(ui.checkBox_border, SIGNAL(toggled(bool)), this, SLOT(SlotBorderShow(bool)));
	}

	CMFloodAnalysisWidget::~CMFloodAnalysisWidget()
	{
	}

	void CMFloodAnalysisWidget::HandleMeasureMsg( EMeasureAction msg, CMeasure* pMeasure )
	{
		if (pMeasure && (pMeasure != m_opFloodAnalysisMeasure.get()))
		{
			m_opFloodAnalysisMeasure = dynamic_cast<CFloodAnalysisMeasure*>(pMeasure);
		}

		if(EM_END == msg || EM_RESULT_CHANGED == msg)
		{
			UpdateDisplay();
		}
	}

	void CMFloodAnalysisWidget::UpdateDisplay()
	{
		if(m_opFloodAnalysisMeasure.valid())
		{
			const FeMeasure::CFloodAnalysisResultData* floodData = 
				m_opFloodAnalysisMeasure->GetCurrentResult()->GetResultData<CFloodAnalysisResultData>();

			if(!floodData) return;

			ui.lineEdit_maxHeiPoint->setText(QString("%1, %2, %3m").arg(
				floodData->maxHeiPoint.x(), 0, 'f').arg(
				floodData->maxHeiPoint.y(), 0, 'f').arg(floodData->maxHeiPoint.z(), 0, 'f'));

			ui.lineEdit_minHeiPoint->setText(QString("%1, %2, %3m").arg(
				floodData->minHeiPoint.x(), 0, 'f').arg(
				floodData->minHeiPoint.y(), 0, 'f').arg(floodData->minHeiPoint.z(), 0, 'f'));

			ui.lineEdit_Area->setText(GetMeasureFormatString(floodData->dTotalArea, 1000000, "m^2", "km^2"));
			ui.lineEdit_floodArea->setText(GetMeasureFormatString(floodData->dFloodArea, 1000000, "m^2", "km^2"));

			ui.doubleSpinBox_deltaHei->setValue(floodData->dDeltaHei);
			ui.checkBox_border->setChecked(m_opFloodAnalysisMeasure->IsShowBorder());
			ui.label_floodHei->setText(QString("%1").arg(floodData->maxHeiPoint.z(), 0, 'f'));
		}
	}

	void CMFloodAnalysisWidget::SlotBeginAnalyze()
	{
		if (m_opFloodAnalysisMeasure.valid())
		{
			m_opFloodAnalysisMeasure->SetDeltaHeight(ui.doubleSpinBox_deltaHei->value());
			m_opFloodAnalysisMeasure->StartDynamicCalculate();
		}
	}

	void CMFloodAnalysisWidget::SlotPauseAnalyze()
	{
		if (m_opFloodAnalysisMeasure.valid())
		{
			m_opFloodAnalysisMeasure->PauseDynamicCalculate();
		}
	}

	void CMFloodAnalysisWidget::SlotReBeginAnalyze()
	{
		if (m_opFloodAnalysisMeasure.valid())
		{
			m_opFloodAnalysisMeasure->SetDeltaHeight(ui.doubleSpinBox_deltaHei->value());
			m_opFloodAnalysisMeasure->ResetFloodState();
			m_opFloodAnalysisMeasure->StartDynamicCalculate();
		}
	}

	void CMFloodAnalysisWidget::SlotBorderShow(bool bShow)
	{
		if (m_opFloodAnalysisMeasure.valid())
		{
			m_opFloodAnalysisMeasure->SetBorderShow(bShow);
		}
	}

}