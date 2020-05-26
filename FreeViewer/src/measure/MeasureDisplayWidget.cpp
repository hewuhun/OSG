#include <measure/DistanceWidget.h>

namespace FreeViewer
{
	void CMeasureDisplayWidget::SendMsg( EMeasureAction msg, CMeasure* pMeasure )
	{
		if(pMeasure)
		{
			m_opMeasure = pMeasure;
			HandleMeasureMsg(msg, pMeasure);

			if(msg == EM_BEGIN)
			{
				hide();
			}
			else if(msg == EM_END)
			{
				ShowDialogNormal();
			}
		}
	}

	void CMeasureDisplayWidget::closeEvent( QCloseEvent * e )
	{
		if (m_opMeasure.valid())
		{
			m_opMeasure->ClearResult();
		}

		CFreeDialog::closeEvent(e);
	}

}
