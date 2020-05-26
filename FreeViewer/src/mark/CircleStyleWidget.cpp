#include <QColorDialog>
#include <FeExtNode/ExPlaceNode.h>
#include <FeExtNode/ExCircleNode.h>
#include <FeUtils/EventListener.h>
#include <mark/Common.h>
#include <mark/CircleStyleWidget.h>

namespace FreeViewer
{
	CCircleStyleWidget::CCircleStyleWidget( FeExtNode::CExCircleNode* opMark, QWidget *parent /*= 0*/ )
		: CBaseStyleWidget(parent)
		,m_opMark(opMark)
	{
		ui.setupUi(this);
		initWidget();
		setWindowTitle(tr("CircleStyle"));
	}

	bool CCircleStyleWidget::Reject()
	{
		if(m_opMark.valid())
		{
			m_opMark->SetLineWidth(m_markOpt.lineWidth());
			m_opMark->SetLineColor(m_markOpt.lineColor());
			m_opMark->SetFillColor(m_markOpt.fillColor());
			m_opMark->SetRadius(m_markOpt.Radius());
			m_opMark->SetPosition(m_markOpt.geoPoint());
		}
		return true;
	}

	void CCircleStyleWidget::initWidget()
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
		
		ui.circleRadiusSpin->setValue(m_opMark->GetRadius());

		FeExtNode::CExCircleNode* pCircle = dynamic_cast<FeExtNode::CExCircleNode*>(m_opMark.get());
		if (pCircle)
		{
			ui.circleTransparantSpin->setValue(pCircle->GetFillColor().a());

			QString strCircleColor;

			strCircleColor = "background-color:rgb(" + QString::number(pCircle->GetFillColor().r() * 255) + "," + QString::number(pCircle->GetFillColor().g() * 255) + ","
				+ QString::number(pCircle->GetFillColor().b() * 255) + ");";
			ui.circleColorBtn->setStyleSheet(strCircleColor);

		}

		connect(ui.lineTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineColorTransparant(double)));

		connect(ui.lineWidthSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineWidth(double)));
		connect(ui.circleTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeCircleColorTransparant(double)));
		connect(ui.lineColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeLineColor()));
		connect(ui.circleColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeCircleColor()));

		connect(ui.clongitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeCircleLongitude(double)));
		connect(ui.clatitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeCircleLatitude(double)));
		connect(ui.cheightSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeHeight(double)));
		
		connect(ui.circleRadiusSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeRadius(double)));
	
		ui.label_12->hide();
		ui.cheightSpin->hide();
		ui.label_hei->hide();
	}

	void CCircleStyleWidget::SlotChangeLineWidth( double dWidth)
	{
		m_opMark->SetLineWidth(dWidth);
	}

	void CCircleStyleWidget::SlotChangeLineColor(  )
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

	void CCircleStyleWidget::SlotChangeLineColorTransparant( double dTransparant)
	{
		if(m_opMark.valid())
		{
			osg::Vec4d vecColor = m_opMark->GetLineColor();
			vecColor.a() = dTransparant;
			m_opMark->SetLineColor(vecColor);
		}
	}

	void CCircleStyleWidget::SlotChangeCircleColor(  )
	{
		QColorDialog* pColorDlg = new QColorDialog(QColor(m_opMark->GetFillColor().r(), m_opMark->GetFillColor().g(), m_opMark->GetFillColor().b(), m_opMark->GetFillColor().a()));
		pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

		if (pColorDlg->exec() == QDialog::Accepted)
		{
			QString strCircleColor;
			strCircleColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
				+ QString::number(pColorDlg->currentColor().blue()) + ");";
			ui.circleColorBtn->setStyleSheet(strCircleColor);

			(dynamic_cast<FeExtNode::CExCircleNode*>(m_opMark.get()))->SetFillColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, (int)ui.lineTransparantSpin->value()));
		}
	}

	void CCircleStyleWidget::SlotChangeCircleColorTransparant( double dTransparant)
	{
		FeExtNode::CExCircleNode* pCircle = dynamic_cast<FeExtNode::CExCircleNode*>(m_opMark.get());
		if(pCircle)
		{
			osg::Vec4d vecColor = pCircle->GetFillColor();
			vecColor.a() = dTransparant;
			pCircle->SetFillColor(vecColor);
		}
	}

	void CCircleStyleWidget::SlotChangeRadius( double dRadius )
	{
		m_opMark->SetRadius(dRadius);
	}

	void CCircleStyleWidget::SlotChangeCircleLongitude( double dLongitude)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.x() = dLongitude;
		m_opMark->SetPosition(npos);
	}

	void CCircleStyleWidget::SlotChangeCircleLatitude( double dLatitude)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.y() = dLatitude;
		m_opMark->SetPosition(npos);
	}

	void CCircleStyleWidget::SlotChangeHeight( double dHeight)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();  
		npos.z() = dHeight;
		m_opMark->SetPosition(npos);
	}

	void CCircleStyleWidget::SlotSDKEventHappend( unsigned int eType)
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
