#include <measure/ContourWidget.h>

#include <mainWindow/FreeUtil.h>
using namespace FreeViewer;

namespace FreeViewer
{
	CContourWidget::CContourWidget( QWidget* parent )
		: CMeasureDisplayWidget(parent)
		, m_opContourMeasure(NULL)
	{
		MEASURE_RESULT_DLG_SETUP_UI(ui);

		connect(ui.pushButton_measure, SIGNAL(pressed()), this, SLOT(SlotBeginMeasure()));
	}

	CContourWidget::~CContourWidget()
	{

	}

	void CContourWidget::HandleMeasureMsg( EMeasureAction msg, CMeasure* pMeasure )
	{
		if (pMeasure != m_opContourMeasure.get())
		{
			m_opContourMeasure = dynamic_cast<CContourMeasure*>(pMeasure);
		}

		if ((EM_END == msg || EM_RESULT_CHANGED == msg) && m_opContourMeasure.valid())
		{
			ui.spinBox_interval->setValue(m_opContourMeasure->GetContourInterval());
			CContourResult *pResultData = dynamic_cast<CContourResult*>(const_cast<CMeasureResult*>(m_opContourMeasure->GetCurrentResult()));
			ui.lineEdit_max->setText(QString::number(pResultData->GetMaxContourValue()));
			ui.lineEdit_min->setText(QString::number(pResultData->GetMinContourValue()));
		}
	}

	void CContourWidget::SlotBeginMeasure()
	{
		if (m_opContourMeasure.valid())
		{
			CContourResult *pResultData = dynamic_cast<CContourResult*>(const_cast<CMeasureResult*>(m_opContourMeasure->GetCurrentResult()));
			if (pResultData)
			{
				 pResultData->SetContourInterval(ui.spinBox_interval->value());
				 pResultData->UpdateContourNode(true);
				 ui.lineEdit_max->setText(QString::number(pResultData->GetMaxContourValue()));
				 ui.lineEdit_min->setText(QString::number(pResultData->GetMinContourValue()));
			}
		}
	}

}