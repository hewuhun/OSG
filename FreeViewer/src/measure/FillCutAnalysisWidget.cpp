#include <measure/FillCutAnalysisWidget.h>

#include <mainWindow/FreeUtil.h>
using namespace FreeViewer;

namespace FreeViewer
{
	CMFillCutAnalysisWidget::CMFillCutAnalysisWidget( QWidget* parent )
		: CMeasureDisplayWidget(parent)
		, m_opFillCutAnalysisMeasure(NULL)
	{
		MEASURE_RESULT_DLG_SETUP_UI(ui);

		connect(ui.pushButton_Analyze, SIGNAL(pressed()), this, SLOT(SlotBeginAnalyze()));
	}

	CMFillCutAnalysisWidget::~CMFillCutAnalysisWidget()
	{

	}

	void CMFillCutAnalysisWidget::HandleMeasureMsg( EMeasureAction msg, CMeasure* pMeasure )
	{
		if (pMeasure != m_opFillCutAnalysisMeasure.get())
		{
			m_opFillCutAnalysisMeasure = dynamic_cast<CFillCutAnalysisMeasure*>(pMeasure);
		}

		if ((EM_END == msg || EM_RESULT_CHANGED == msg) && m_opFillCutAnalysisMeasure.valid())
		{
			const CFillCutAnalysisResultData* resultData = 
				m_opFillCutAnalysisMeasure.get()->GetCurrentResult()->GetResultData<CFillCutAnalysisResultData>();

			if(resultData)
			{
				ui.lineEdit_maxHeiPoint->setText(QString("%1, %2, %3m").arg(
					resultData->vecMaxHeiPoint.x(), 0, 'f').arg(
					resultData->vecMaxHeiPoint.y(), 0, 'f').arg(resultData->vecMaxHeiPoint.z(), 0, 'f'));

				ui.lineEdit_minHeiPoint->setText(QString("%1, %2, %3m").arg(
					resultData->vecMinHeiPoint.x(), 0, 'f').arg(
					resultData->vecMinHeiPoint.y(), 0, 'f').arg(resultData->vecMinHeiPoint.z(), 0, 'f'));

				ui.lineEdit_fillVolume->setText(GetMeasureFormatString(resultData->dFillVolume, 1000000000, "m^3", "km^3"));
				ui.lineEdit_cutVolume->setText(GetMeasureFormatString(resultData->dCutVolune, 1000000000, "m^3", "km^3"));
				ui.lineEdit_totalVolume->setText(GetMeasureFormatString(resultData->dFillVolume + resultData->dCutVolune, 1000000000, "m^3", "km^3"));
				ui.lineEdit_fillArea->setText(GetMeasureFormatString(resultData->dFillArea, 1000000, "m^2", "km^2"));
				ui.lineEdit_cutArea->setText(GetMeasureFormatString(resultData->dCutArea, 1000000, "m^2", "km^2"));
				ui.lineEdit_totalArea->setText(GetMeasureFormatString(resultData->dFillArea + resultData->dCutArea, 1000000, "m^2", "km^2"));
				ui.doubleSpinBox_refHei->setValue(resultData->dReferenHei);
			}
		}
	}

	void CMFillCutAnalysisWidget::SlotBeginAnalyze()
	{
		if (m_opFillCutAnalysisMeasure.valid())
		{
			const CFillCutAnalysisResultData* resultData = 
				m_opFillCutAnalysisMeasure.get()->GetCurrentResult()->GetResultData<CFillCutAnalysisResultData>();

			if(resultData) 
			{
				if (ui.doubleSpinBox_refHei->value() != resultData->dReferenHei)
				{
					m_opFillCutAnalysisMeasure->SetReferenceHeight(ui.doubleSpinBox_refHei->value());
				}
			}
		}
	}

}