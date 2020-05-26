#include <measure/BuildingsViewWidget.h>
#include <QColorDialog>

namespace FreeViewer
{
	CMBuildingsViewWidget::CMBuildingsViewWidget( QWidget* parent )
		: CMeasureDisplayWidget(parent)
	{
		MEASURE_RESULT_DLG_SETUP_UI(ui);
		
		connect(ui.ColorBtn, SIGNAL(clicked()), SLOT(SlotLineColorChanged()));

		connect(ui.doubleSpinBox_Lon, SIGNAL(valueChanged(double)), SLOT(SlotPositionXChanged(double)));
		connect(ui.doubleSpinBox_Lat, SIGNAL(valueChanged(double)), SLOT(SlotPositionYChanged(double)));
		connect(ui.doubleSpinBox_Hight, SIGNAL(valueChanged(double)), SLOT(SlotPositionZChanged(double)));

		connect(ui.farClipSpin, SIGNAL(valueChanged(double)), SLOT(SlotFarClipChanged(double)));
		connect(ui.nearClipSpin, SIGNAL(valueChanged(double)), SLOT(SlotNearClipChanged(double)));
		connect(ui.fovSpin, SIGNAL(valueChanged(double)), SLOT(SlotFovChanged(double)));
		connect(ui.fovAspectSpin, SIGNAL(valueChanged(double)), SLOT(SlotFovAspectChanged(double)));
		connect(ui.pitchSpin, SIGNAL(valueChanged(double)), SLOT(SlotPitchAngleChanged(double)));
	}

	CMBuildingsViewWidget::~CMBuildingsViewWidget()
	{

	}

	void CMBuildingsViewWidget::HandleMeasureMsg( EMeasureAction msg, CMeasure* pMeasure )
	{
		if(m_opBuildingViewMeasure.get() != pMeasure)
		{
			m_opBuildingViewMeasure = dynamic_cast<CBuildingsViewMeasure*>(pMeasure);
		}

		if(msg == EM_UPDATE || msg == EM_RESULT_CHANGED)
		{
			if(m_opBuildingViewMeasure.valid())
			{
				osg::Vec3d vecPosLLH = m_opBuildingViewMeasure.get()->GetLookPosition();
				ui.doubleSpinBox_Lon->setValue(vecPosLLH.x());
				ui.doubleSpinBox_Lat->setValue(vecPosLLH.y());
				ui.doubleSpinBox_Hight->setValue(vecPosLLH.z());

				ui.farClipSpin->setValue(m_opBuildingViewMeasure.get()->GetFarClip());
				ui.nearClipSpin->setValue(m_opBuildingViewMeasure.get()->GetNearClip());
				ui.fovSpin->setValue(m_opBuildingViewMeasure.get()->GetFov());
				ui.fovAspectSpin->setValue(m_opBuildingViewMeasure.get()->GetFovAspect());
				ui.pitchSpin->setValue(m_opBuildingViewMeasure.get()->GetPitchAngle());

				osg::Vec4 vecColor = m_opBuildingViewMeasure.get()->GetFrustumLineColor();
				QString strLineColor = "background-color:rgb(" + QString::number(vecColor.r() * 255) 
					+ "," + QString::number(vecColor.g() * 255) + ","+ QString::number(vecColor.b() * 255) + ");";

				ui.ColorBtn->setStyleSheet(strLineColor);
			}
		}
		
	}

	void CMBuildingsViewWidget::SlotPositionXChanged( double val )
	{
		if(m_opBuildingViewMeasure.valid())
		{
			osg::Vec3d vecPosLLH = m_opBuildingViewMeasure.get()->GetLookPosition();
			vecPosLLH.x() = val;
			m_opBuildingViewMeasure.get()->SetLookPosition(vecPosLLH);
		}
	}

	void CMBuildingsViewWidget::SlotPositionYChanged( double val )
	{
		if(m_opBuildingViewMeasure.valid())
		{
			osg::Vec3d vecPosLLH = m_opBuildingViewMeasure.get()->GetLookPosition();
			vecPosLLH.y() = val;
			m_opBuildingViewMeasure.get()->SetLookPosition(vecPosLLH);
		}
	}

	void CMBuildingsViewWidget::SlotPositionZChanged( double val )
	{
		if(m_opBuildingViewMeasure.valid())
		{
			osg::Vec3d vecPosLLH = m_opBuildingViewMeasure.get()->GetLookPosition();
			vecPosLLH.z() = val;
			m_opBuildingViewMeasure.get()->SetLookPosition(vecPosLLH);
		}
	}

	void CMBuildingsViewWidget::SlotNearClipChanged( double val )
	{
		if(m_opBuildingViewMeasure.valid())
		{
			m_opBuildingViewMeasure.get()->SetNearClip(val);
		}
	}

	void CMBuildingsViewWidget::SlotFarClipChanged( double val )
	{
		if(m_opBuildingViewMeasure.valid())
		{
			m_opBuildingViewMeasure.get()->SetFarClip(val);
		}
	}

	void CMBuildingsViewWidget::SlotFovChanged( double val )
	{
		if(m_opBuildingViewMeasure.valid())
		{
			m_opBuildingViewMeasure.get()->SetFov(val);
		}
	}

	void CMBuildingsViewWidget::SlotFovAspectChanged( double val )
	{
		if(m_opBuildingViewMeasure.valid())
		{
			m_opBuildingViewMeasure.get()->SetFovAspect(val);
		}
	}

	void CMBuildingsViewWidget::SlotPitchAngleChanged( double val )
	{
		if(m_opBuildingViewMeasure.valid())
		{
			m_opBuildingViewMeasure.get()->SetPitchAngle(val);
		}
	}

	void CMBuildingsViewWidget::SlotLineColorChanged()
	{
		if(m_opBuildingViewMeasure.valid())
		{
			osg::Vec4 vecColor = m_opBuildingViewMeasure.get()->GetFrustumLineColor();

			QColorDialog* pColorDlg = new QColorDialog(QColor(vecColor.r(), vecColor.g(), vecColor.b(), vecColor.a()));

			if (pColorDlg->exec() == QDialog::Accepted)
			{
				QString strLineColor;
				strLineColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," 
					+ QString::number(pColorDlg->currentColor().green()) + ","
					+ QString::number(pColorDlg->currentColor().blue()) + ");";

				ui.ColorBtn->setStyleSheet(strLineColor);

				m_opBuildingViewMeasure.get()->SetFrustumLineColor(
					osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, 
					pColorDlg->currentColor().blue() / 255.0, 1));
			}
		}
	}

}