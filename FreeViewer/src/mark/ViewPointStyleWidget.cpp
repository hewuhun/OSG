#include <mark/ViewPointStyleWidget.h>
#include <FeExtNode/ExLodNode.h>
#include <FeExtNode/ExLodModelNode.h>
#include <FeUtils/CoordConverter.h>

namespace FreeViewer
{

	CViewPointStyleWidget::CViewPointStyleWidget( 
		FeShell::CSystemService* pSystemService,
		FeExtNode::CExternNode* pExternNode, 
		bool bCreate,
		QWidget *parent /*= 0*/ )
		:CBaseStyleWidget(parent)
		,m_opExNode(pExternNode)
		,m_opSystemService(pSystemService)
		,m_bCreate(bCreate)
		,m_bViewPointChanged(false)
	{
		ui.setupUi(this);
		setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

		BuildContext();

		connect(ui.current_view_toolButton, SIGNAL(clicked(bool)),
			this, SLOT(SlotCurrentView(bool)));
		connect(ui.reset_toolButton, SIGNAL(clicked(bool)),
			this, SLOT(SlotResetView(bool)));
		connect(ui.doubleSpinBox_rangeLow, SIGNAL(valueChanged(double)), 
			this, SLOT(SlotChangeRangeLow(double)));
		connect(ui.doubleSpinBox_rangeHigh, SIGNAL(valueChanged(double)), 
			this, SLOT(SlotChangeRangeHigh(double)));
		connect(ui.latitude_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SlotUiChanged(double)));
		connect(ui.lontitude_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SlotUiChanged(double)));
		connect(ui.height_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SlotUiChanged(double)));
		connect(ui.fly_time_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SlotUiChanged(double)));
		connect(ui.pitch_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SlotUiChanged(double)));
		connect(ui.heading_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SlotUiChanged(double)));
	}

	CViewPointStyleWidget::~CViewPointStyleWidget()
	{

	}

	void CViewPointStyleWidget::BuildContext()
	{
		if(m_opExNode.valid())
		{
			setWindowTitle(tr("ViewPoint"));
			m_viewPoint = m_opExNode->GetViewPoint();
			SetViewPoint(m_viewPoint);

			FeExtNode::CExLodNode* lodNode = dynamic_cast<FeExtNode::CExLodNode*>(m_opExNode.get());
			if(lodNode)
			{
				FeExtNode::CExLodModelNode* pLodModelNode = dynamic_cast<FeExtNode::CExLodModelNode*>(m_opExNode.get());
				if (pLodModelNode)
				{
					ui.groupBox_viewRange->hide();
				}
				else
				{
					lodNode->GetRange(m_dRangeLow, m_dRangeHigh);
					ui.doubleSpinBox_rangeLow->setValue(m_dRangeLow);
					ui.doubleSpinBox_rangeHigh->setValue(m_dRangeHigh);
				}
			}
			else
			{
				ui.groupBox_viewRange->hide();
			}
		}
	}

	void CViewPointStyleWidget::SetViewPoint(const FeUtil::CFreeViewPoint& freeViewPoint)
	{
		double lon = freeViewPoint.GetLon();
		ui.lontitude_doubleSpinBox->setValue(freeViewPoint.GetLon());
		lon = ui.lontitude_doubleSpinBox->value();

		ui.latitude_doubleSpinBox->setValue(freeViewPoint.GetLat());
		ui.height_doubleSpinBox->setValue(freeViewPoint.GetHei());
		ui.heading_doubleSpinBox->setValue(freeViewPoint.GetHeading());
		ui.pitch_doubleSpinBox->setValue(freeViewPoint.GetPitch());
		ui.fly_time_doubleSpinBox->setValue(freeViewPoint.GetTime());
	}

	void CViewPointStyleWidget::SlotCurrentView(bool)
	{

		//test
		//FeUtil::CFreeViewPoint vp = m_opSystemService->GetViewPoint();
		//osg::Vec3d eye(0,0,0);
		//osg::Vec3d center(0,0,0);
		//osg::Vec3d up(0,0,0);
		//osg::Vec3d llh(0,0,0);
		//m_opSystemService->GetRenderContext()->GetCamera()->getViewMatrixAsLookAt(eye, center, up);
		//FeUtil::XYZ2DegreeLLH(m_opSystemService->GetRenderContext(), eye, llh);
		//

		SetViewPoint(m_opSystemService->GetViewPoint());
		m_bViewPointChanged = true;
	}

	void CViewPointStyleWidget::SlotResetView(bool)
	{
		FeUtil::CFreeViewPoint viewPoint;
		SetViewPoint(viewPoint);

		m_bViewPointChanged = true;
	}

	void CViewPointStyleWidget::SlotChangeRangeLow(double dValue)
	{
		double dHigh = ui.doubleSpinBox_rangeHigh->text().toDouble();
		if (dValue > dHigh)
		{
			dValue = dHigh;
			ui.doubleSpinBox_rangeLow->setValue(dHigh);
		}

		FeExtNode::CExLodNode* lodNode = dynamic_cast<FeExtNode::CExLodNode*>(m_opExNode.get());
		if(lodNode)
		{
			double dRangeLow, dRangeHigh;  
			lodNode->GetRange(dRangeLow,dRangeHigh);
			lodNode->SetRange(dValue, dRangeHigh);
		}

		m_bViewPointChanged = true;
	}

	void CViewPointStyleWidget::SlotChangeRangeHigh(double  dValue)
	{
		double dLow = ui.doubleSpinBox_rangeLow->text().toDouble();
		if (dValue <= dLow)
		{
			dLow = dValue;
			ui.doubleSpinBox_rangeLow->setValue(dLow);
		}

		FeExtNode::CExLodNode* lodNode = dynamic_cast<FeExtNode::CExLodNode*>(m_opExNode.get());
		if(lodNode)
		{
			double dRangeLow, dRangeHigh;  
			lodNode->GetRange(dRangeLow,dRangeHigh);
			lodNode->SetRange(dRangeLow, dValue);
		}

		m_bViewPointChanged = true;
	}

	bool CViewPointStyleWidget::Accept()
	{
		if(m_opExNode.valid())
		{
			double dLon = ui.lontitude_doubleSpinBox->value();
			double dLat = ui.latitude_doubleSpinBox->value();
			double dHei = ui.height_doubleSpinBox->value();
			double dHeading = ui.heading_doubleSpinBox->value();
			double dPitch = ui.pitch_doubleSpinBox->value();
			double dFlyTime = ui.fly_time_doubleSpinBox->value();
			double dRangeLow = ui.doubleSpinBox_rangeLow->value();
			double dRangeHigh = ui.doubleSpinBox_rangeHigh->value();

			FeUtil::CFreeViewPoint viewPoint(dLon, dLat, dHei, dHeading, dPitch, m_opSystemService->GetViewPoint().GetRange(), dFlyTime);
			m_opExNode->SetViewPoint(viewPoint);
		}
		return CBaseStyleWidget::Accept();
	}

	bool CViewPointStyleWidget::Reject()
	{
		if(m_opExNode.valid())
		{
			m_opExNode->SetViewPoint(m_viewPoint);

			FeExtNode::CExLodNode* lodNode = dynamic_cast<FeExtNode::CExLodNode*>(m_opExNode.get());
			if(lodNode)
			{
				lodNode->SetRange(m_dRangeLow, m_dRangeHigh);
			}
		}
		return CBaseStyleWidget::Reject();
	}

	void CViewPointStyleWidget::SlotUiChanged(double value)
	{
		m_bViewPointChanged = true;
	}

	void CViewPointStyleWidget::SlotChangedViwePointByMode(CFreeViewPoint freeViewPoint)
	{
		if (!m_bViewPointChanged)
		{
			ui.lontitude_doubleSpinBox->blockSignals(true);
			ui.latitude_doubleSpinBox->blockSignals(true);
			ui.height_doubleSpinBox->blockSignals(true);
			ui.heading_doubleSpinBox->blockSignals(true);
			ui.pitch_doubleSpinBox->blockSignals(true);
			ui.fly_time_doubleSpinBox->blockSignals(true);

			ui.lontitude_doubleSpinBox->setValue(freeViewPoint.GetLon());
			ui.latitude_doubleSpinBox->setValue(freeViewPoint.GetLat());
			ui.height_doubleSpinBox->setValue(freeViewPoint.GetHei());
			ui.heading_doubleSpinBox->setValue(freeViewPoint.GetHeading());
			ui.pitch_doubleSpinBox->setValue(freeViewPoint.GetPitch());
			ui.fly_time_doubleSpinBox->setValue(freeViewPoint.GetTime());

			ui.lontitude_doubleSpinBox->blockSignals(false);
			ui.latitude_doubleSpinBox->blockSignals(false);
			ui.height_doubleSpinBox->blockSignals(false);
			ui.heading_doubleSpinBox->blockSignals(false);
			ui.pitch_doubleSpinBox->blockSignals(false);
			ui.fly_time_doubleSpinBox->blockSignals(false);
		}
	}

}