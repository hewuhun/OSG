#include <QColorDialog>
#include <FeExtNode/ExPlaceNode.h>
#include <FeExtNode/ExEllipseNode.h>
#include <FeUtils/EventListener.h>
#include <mark/EllipseStyleWidget.h>

namespace FreeViewer
{
	CEllipseStyleWidget::CEllipseStyleWidget( FeExtNode::CExEllipseNode* opMark, QWidget *parent /*= 0*/ )
		: CBaseStyleWidget(parent)
		,m_opMark(opMark)
	{
		ui.setupUi(this);
		initWidget();
		setWindowTitle(tr("EllipseStyle"));
	}

	bool CEllipseStyleWidget::Reject()
	{
		if(m_opMark.valid())
		{
			m_opMark->SetLineWidth(m_markOpt.lineWidth());
			m_opMark->SetLineColor(m_markOpt.lineColor());
			m_opMark->SetFillColor(m_markOpt.fillColor());
			m_opMark->SetPosition(m_markOpt.geoPoint());
			m_opMark->SetLongRadius(m_markOpt.LongRadiu());
			m_opMark->SetShortRadius(m_markOpt.ShortRadiu());
		}
		return true;
	}

	void CEllipseStyleWidget::initWidget()
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
		
		ui.ellipseLongSpin->setValue(m_opMark->GetLongRadius());
		ui.ellipseShortSpin->setValue(m_opMark->GetShortRadius());

		FeExtNode::CExEllipseNode* pEllipse = dynamic_cast<FeExtNode::CExEllipseNode*>(m_opMark.get());
		if (pEllipse)
		{
			ui.ellipseTransparantSpin->setValue(pEllipse->GetFillColor().a());

			QString strEllipseColor;

			strEllipseColor = "background-color:rgb(" + QString::number(pEllipse->GetFillColor().r() * 255) + "," + QString::number(pEllipse->GetFillColor().g() * 255) + ","
				+ QString::number(pEllipse->GetFillColor().b() * 255) + ");";
			ui.ellipseColorBtn->setStyleSheet(strEllipseColor);

		}

		connect(ui.lineTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineColorTransparant(double)));

		connect(ui.lineWidthSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineWidth(double)));
		connect(ui.ellipseTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeEllipseColorTransparant(double)));
		connect(ui.lineColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeLineColor()));
		connect(ui.ellipseColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeEllipseColor()));

		connect(ui.clongitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeEllipseLongitude(double)));
		connect(ui.clatitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeEllipseLatitude(double)));
		connect(ui.cheightSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeHeight(double)));

		connect(ui.ellipseLongSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeEllipselRadius(double)));
		connect(ui.ellipseShortSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeEllipsesRadius(double)));

		ui.label_12->hide();
		ui.cheightSpin->hide();
		ui.label_hei->hide();
	}


	void CEllipseStyleWidget::SlotChangeLineWidth( double dWidth)
	{
		m_opMark->SetLineWidth(dWidth);
	}

	void CEllipseStyleWidget::SlotChangeLineColor(  )
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

	void CEllipseStyleWidget::SlotChangeLineColorTransparant( double dTransparant)
	{
		if(m_opMark.valid())
		{
			osg::Vec4d vecColor = m_opMark->GetLineColor();
			vecColor.a() = dTransparant;
			m_opMark->SetLineColor(vecColor);
		}
	}

	void CEllipseStyleWidget::SlotChangeEllipseColor(  )
	{
		QColorDialog* pColorDlg = new QColorDialog(QColor(m_opMark->GetFillColor().r(), m_opMark->GetFillColor().g(), m_opMark->GetFillColor().b(), m_opMark->GetFillColor().a()));
		pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

		if (pColorDlg->exec() == QDialog::Accepted)
		{
			QString strArcColor;
			strArcColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
				+ QString::number(pColorDlg->currentColor().blue()) + ");";
			ui.ellipseColorBtn->setStyleSheet(strArcColor);

			(dynamic_cast<FeExtNode::CExEllipseNode*>(m_opMark.get()))->SetFillColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, (int)ui.lineTransparantSpin->value()));
		}
	}

	void CEllipseStyleWidget::SlotChangeEllipseColorTransparant( double dTransparant)
	{
		FeExtNode::CExEllipseNode* pEllipse = dynamic_cast<FeExtNode::CExEllipseNode*>(m_opMark.get());
		if(pEllipse)
		{
			osg::Vec4d vecColor = pEllipse->GetFillColor();
			vecColor.a() = dTransparant;
			pEllipse->SetFillColor(vecColor);
		}
	}


	void CEllipseStyleWidget::SlotChangeEllipselRadius( double longRadius)
	{
		m_opMark->SetLongRadius(longRadius);
	}

	void CEllipseStyleWidget::SlotChangeEllipsesRadius( double shortRadius)
	{
		m_opMark->SetShortRadius(shortRadius);
	}

	void CEllipseStyleWidget::SlotChangeEllipseLongitude( double dLongitude)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.x() = dLongitude;
		m_opMark->SetPosition(npos);
	}

	void CEllipseStyleWidget::SlotChangeEllipseLatitude( double dLatitude)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.y() = dLatitude;
		m_opMark->SetPosition(npos);
	}

	void CEllipseStyleWidget::SlotChangeHeight( double dHeight)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.z() = dHeight;
		m_opMark->SetPosition(npos);
	}

	void CEllipseStyleWidget::SlotSDKEventHappend( unsigned int eType)
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
