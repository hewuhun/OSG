#include <QColorDialog>
#include <FeExtNode/ExPlaceNode.h>
#include <FeExtNode/ExSectorNode.h>
#include <FeUtils/EventListener.h>
#include <mark/Common.h>
#include <mark/SectorStyleWidget.h>

namespace FreeViewer
{
	CSectorStyleWidget::CSectorStyleWidget( FeExtNode::CExSectorNode* opMark, QWidget *parent /*= 0*/ )
		: CBaseStyleWidget(parent)
		,m_opMark(opMark)
	{
		ui.setupUi(this);
		initWidget();
		setWindowTitle(tr("SectorStyle"));
	}

	bool CSectorStyleWidget::Reject()
	{
		if(m_opMark.valid())
		{
			m_opMark->SetLineWidth(m_markOpt.lineWidth());
			m_opMark->SetLineColor(m_markOpt.lineColor());
			m_opMark->SetFillColor(m_markOpt.fillColor());
			m_opMark->SetRadius(m_markOpt.Radius());
			m_opMark->SetPosition(m_markOpt.geoPoint());
			m_opMark->SetArcStart(m_markOpt.ArcStart());
			m_opMark->SetArcEnd(m_markOpt.ArcEnd());
		}
		return true;
	}

	void CSectorStyleWidget::initWidget()
	{
		m_markOpt = *(m_opMark->GetOption());

		QString strLineColor;
		strLineColor = "background-color:rgb(" 
			+ QString::number(m_opMark->GetLineColor().r() * 255) + ","
			+ QString::number(m_opMark->GetLineColor().g() * 255) + ","
			+ QString::number(m_opMark->GetLineColor().b() * 255) + ");";
		ui.lineColorBtn->setStyleSheet(strLineColor);

		ui.lineWidthSpin->setValue(m_opMark->GetLineWidth());
		ui.lineTransparantSpin->setValue(m_opMark->GetLineColor().a());
		
		//标记属性界面初始化
		osgEarth::GeoPoint vpos = m_opMark->GetPosition();	
		ui.clongitudeSpin->setValue(vpos.x());
		ui.clatitudeSpin->setValue(vpos.y());
		ui.cheightSpin->setValue(vpos.z());
		
		ui.sectorStartSpin->setValue(m_opMark->GetArcStart());
		ui.sectorEndSpin->setValue(m_opMark->GetArcEnd());
		ui.sectorRadiusSpin->setValue(m_opMark->GetRadius());

		FeExtNode::CExSectorNode* pSector = dynamic_cast<FeExtNode::CExSectorNode*>(m_opMark.get());
		if (pSector)
		{
			ui.sectorTransparantSpin->setValue(pSector->GetFillColor().a());

			QString strSectorColor;

			strSectorColor = "background-color:rgb(" + QString::number(pSector->GetFillColor().r() * 255) + "," + QString::number(pSector->GetFillColor().g() * 255) + ","
				+ QString::number(pSector->GetFillColor().b() * 255) + ");";
			ui.sectorColorBtn->setStyleSheet(strSectorColor);

		}

		connect(ui.lineTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineColorTransparant(double)));

		connect(ui.lineWidthSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineWidth(double)));
		connect(ui.sectorTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeSectorColorTransparant(double)));
		connect(ui.lineColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeLineColor()));
		connect(ui.sectorColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeSectorColor()));

		connect(ui.clongitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeSectorLongitude(double)));
		connect(ui.clatitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeSectorLatitude(double)));
		connect(ui.cheightSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeHeight(double)));

		connect(ui.sectorStartSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeSectorStart(double)));
		connect(ui.sectorEndSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeSectorEnd(double)));
		connect(ui.sectorRadiusSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeRadius(double)));

		ui.label_12->hide();
		ui.cheightSpin->hide();
		ui.label_hei->hide();
	}

	void CSectorStyleWidget::SlotChangeLineWidth( double dWidth)
	{
		m_opMark->SetLineWidth(dWidth);
	}

	void CSectorStyleWidget::SlotChangeLineColor(  )
	{
		QColorDialog* pColorDlg = new QColorDialog(QColor(m_opMark->GetLineColor().r(), m_opMark->GetLineColor().g(), m_opMark->GetLineColor().b(), m_opMark->GetLineColor().a()));
		pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);


		if (pColorDlg->exec() == QDialog::Accepted)
		{
			QString strLineColor;
			strLineColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
				+ QString::number(pColorDlg->currentColor().blue()) + ");";
			ui.lineColorBtn->setStyleSheet(strLineColor);

			m_opMark->SetLineColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, ui.lineTransparantSpin->value()));
		}
	}

	void CSectorStyleWidget::SlotChangeLineColorTransparant( double dTransparant)
	{
		if(m_opMark.valid())
		{
			osg::Vec4d vecColor = m_opMark->GetLineColor();
			vecColor.a() = dTransparant;
			m_opMark->SetLineColor(vecColor);
		}
	}

	void CSectorStyleWidget::SlotChangeSectorColor(  )
	{
		QColorDialog* pColorDlg = new QColorDialog(QColor(m_opMark->GetFillColor().r(), m_opMark->GetFillColor().g(), m_opMark->GetFillColor().b(), m_opMark->GetFillColor().a()));
		pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);
		
		if (pColorDlg->exec() == QDialog::Accepted)
		{
			QString strArcColor;
			strArcColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
				+ QString::number(pColorDlg->currentColor().blue()) + ");";
			ui.sectorColorBtn->setStyleSheet(strArcColor);

			(dynamic_cast<FeExtNode::CExSectorNode*>(m_opMark.get()))->SetFillColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, (int)ui.lineTransparantSpin->value()));
		}
	}

	void CSectorStyleWidget::SlotChangeSectorColorTransparant( double dTransparant)
	{
		FeExtNode::CExSectorNode* pSector = dynamic_cast<FeExtNode::CExSectorNode*>(m_opMark.get());
		if(pSector)
		{
			osg::Vec4d vecColor = pSector->GetFillColor();
			vecColor.a() = dTransparant;
			pSector->SetFillColor(vecColor);
		}
	}


	void CSectorStyleWidget::SlotChangeSectorStart(double dStart )
	{
		m_opMark->SetArcStart(dStart);
	}

	void CSectorStyleWidget::SlotChangeSectorEnd( double dEnd )
	{
		m_opMark->SetArcEnd(dEnd);
	}

	void CSectorStyleWidget::SlotChangeRadius( double dRadius )
	{
		m_opMark->SetRadius(dRadius);
	}

	void CSectorStyleWidget::SlotChangeSectorLongitude( double dLongitude)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.x() = dLongitude;
		m_opMark->SetPosition(npos);
	}

	void CSectorStyleWidget::SlotChangeSectorLatitude( double dLatitude)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.y() = dLatitude;
		m_opMark->SetPosition(npos);
	}

	void CSectorStyleWidget::SlotChangeHeight( double dHeight)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();  
		npos.z() = dHeight;
		m_opMark->SetPosition(npos);
	}

	void CSectorStyleWidget::SlotSDKEventHappend( unsigned int eType)
	{
		if(!m_opMark.valid()) return;

		if(FeUtil::E_EVENT_TYPE_OBJECT_POSITION_CHANGED == eType)
		{
			osgEarth::GeoPoint vpos = m_opMark->GetPosition();	
			ui.clongitudeSpin->setValue(vpos.x());
			ui.clatitudeSpin->setValue(vpos.y());
			ui.cheightSpin->setValue(vpos.z());
		}
	}

}
