#include <QColorDialog>
#include <FeExtNode/ExPlaceNode.h>
#include <FeUtils/EventListener.h>
#include <mark/ArcStyleWidget.h>

namespace FreeViewer
{
	CArcStyleWidget::CArcStyleWidget( FeExtNode::CExArcNode* opMark, QWidget *parent /*= 0*/ )
		: CBaseStyleWidget(parent)
		,m_opMark(opMark) 
	{
		ui.setupUi(this);
		initWidget();
		setWindowTitle(tr("ArcStyle"));
	}

	bool CArcStyleWidget::Reject()
	{
		if(m_opMark.valid())
		{
			m_opMark->SetLineWidth(m_markOpt.lineWidth());
			m_opMark->SetLineColor(m_markOpt.lineColor());
			m_opMark->SetFillColor(m_markOpt.fillColor());
			m_opMark->SetPosition(m_markOpt.geoPoint());
			m_opMark->SetArcStart(m_markOpt.ArcStart());
			m_opMark->SetArcEnd(m_markOpt.ArcEnd());
			m_opMark->SetRadius(m_markOpt.Radius());
		}
		return true;
	}

	void CArcStyleWidget::initWidget()
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
		
		ui.arcStartSpin->setValue(m_opMark->GetArcStart());
		ui.arcEndSpin->setValue(m_opMark->GetArcEnd());
		ui.arcRadiusSpin->setValue(m_opMark->GetRadius());

		FeExtNode::CExArcNode* pArc = dynamic_cast<FeExtNode::CExArcNode*>(m_opMark.get());
		if (pArc)
		{
			ui.ArcTransparantSpin->setValue(pArc->GetFillColor().a());

			QString strArcColor;

			strArcColor = "background-color:rgb(" + QString::number(pArc->GetFillColor().r() * 255) + "," + QString::number(pArc->GetFillColor().g() * 255) + ","
				+ QString::number(pArc->GetFillColor().b() * 255) + ");";
			ui.ArcColorBtn->setStyleSheet(strArcColor);

		}

		connect(ui.lineTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineColorTransparant(double)));

		connect(ui.lineWidthSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineWidth(double)));
		connect(ui.ArcTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeArcColorTransparant(double)));
		connect(ui.lineColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeLineColor()));
		connect(ui.ArcColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeArcColor()));

		connect(ui.clongitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeArcLongitude(double)));
		connect(ui.clatitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeArcLatitude(double)));
		connect(ui.cheightSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeHeight(double)));

		connect(ui.arcStartSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeArcStart(double)));
		connect(ui.arcEndSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeArcEnd(double)));
		connect(ui.arcRadiusSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeRadius(double)));

		ui.label_8->hide();
		ui.cheightSpin->hide();
		ui.label_hei->hide();
	}

	void CArcStyleWidget::SlotChangeLineWidth( double dWidth)
	{
		m_opMark->SetLineWidth(dWidth);
	}

	void CArcStyleWidget::SlotChangeLineColor(  )
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

	void CArcStyleWidget::SlotChangeLineColorTransparant( double dTransparant)
	{
		if(m_opMark.valid())
		{
			osg::Vec4d vecColor = m_opMark->GetLineColor();
			vecColor.a() = dTransparant;
			m_opMark->SetLineColor(vecColor);
		}
	}

	void CArcStyleWidget::SlotChangeArcColor(  )
	{
		QColorDialog* pColorDlg = new QColorDialog(QColor(m_opMark->GetFillColor().r(), m_opMark->GetFillColor().g(), m_opMark->GetFillColor().b(), m_opMark->GetFillColor().a()));
		pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

		if (pColorDlg->exec() == QDialog::Accepted)
		{
			QString strArcColor;
			strArcColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
				+ QString::number(pColorDlg->currentColor().blue()) + ");";
			ui.ArcColorBtn->setStyleSheet(strArcColor);

			(dynamic_cast<FeExtNode::CExArcNode*>(m_opMark.get()))->SetFillColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, ui.lineTransparantSpin->value()));
		}
	}

	void CArcStyleWidget::SlotChangeArcColorTransparant( double dTransparant)
	{
		FeExtNode::CExArcNode* pArc = dynamic_cast<FeExtNode::CExArcNode*>(m_opMark.get());
		if(pArc)
		{
			osg::Vec4d vecColor = pArc->GetFillColor();
			vecColor.a() = dTransparant;
			pArc->SetFillColor(vecColor);
		}
	}

	void CArcStyleWidget::SlotChangeArcStart(double dStart )
	{
		m_opMark->SetArcStart(dStart);
	}
	 
	void CArcStyleWidget::SlotChangeArcEnd( double dEnd )
	{
		m_opMark->SetArcEnd(dEnd);
	}

	void CArcStyleWidget::SlotChangeRadius( double dRadius )
	{
		m_opMark->SetRadius(dRadius);
	}

	void CArcStyleWidget::SlotChangeArcLongitude( double dLongitude)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.x() = dLongitude;
		m_opMark->SetPosition(npos);
	}

	void CArcStyleWidget::SlotChangeArcLatitude( double dLatitude)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.y() = dLatitude;
		m_opMark->SetPosition(npos);
	}

	void CArcStyleWidget::SlotChangeHeight( double dHeight)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.z() = dHeight;
		m_opMark->SetPosition(npos);
	}

	void CArcStyleWidget::SlotSDKEventHappend( unsigned int eType)
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
