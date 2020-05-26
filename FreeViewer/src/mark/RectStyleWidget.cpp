#include <QColorDialog>
#include <FeExtNode/ExPlaceNode.h>
#include <FeExtNode/ExPolygonNode.h>
#include <FeUtils/EventListener.h>
#include <mark/RectStyleWidget.h>

namespace FreeViewer
{
	CRectStyleWidget::CRectStyleWidget( FeExtNode::CExRectNode* opMark, QWidget *parent /*= 0*/ )
		: CBaseStyleWidget(parent)
		,m_opMark(opMark)
	{
		ui.setupUi(this);
		initWidget();
		setWindowTitle(tr("RectStyle"));
	}

	bool CRectStyleWidget::Reject()
	{
		if(m_opMark.valid())
		{
			m_opMark->SetLineWidth(m_markOpt.lineWidth());
			m_opMark->SetLineColor(m_markOpt.lineColor());
			m_opMark->SetFillColor(m_markOpt.fillColor());
			m_opMark->SetPosition(m_markOpt.geoPoint());
			m_opMark->SetWidth(m_markOpt.Width());
			m_opMark->SetHeight(m_markOpt.Height());
		}
		return true;
	}

	void CRectStyleWidget::initWidget()
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
			
		ui.rectLengthSpin->setValue(m_opMark->GetHeight());
		ui.rectWidthSpin->setValue(m_opMark->GetWidth());

		FeExtNode::CExRectNode* pRect = dynamic_cast<FeExtNode::CExRectNode*>(m_opMark.get());
		if (pRect)
		{
			ui.rectTransparantSpin->setValue(pRect->GetFillColor().a());

			QString strRectColor;

			strRectColor = "background-color:rgb(" + QString::number(pRect->GetFillColor().r() * 255) + "," + QString::number(pRect->GetFillColor().g() * 255) + ","
				+ QString::number(pRect->GetFillColor().b() * 255) + ");";
			ui.rectColorBtn->setStyleSheet(strRectColor);

		}

		connect(ui.lineTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineColorTransparant(double)));

		connect(ui.lineWidthSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineWidth(double)));
		connect(ui.rectTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeRectColorTransparant(double)));
		connect(ui.lineColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeLineColor()));
		connect(ui.rectColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeRectColor()));

		connect(ui.clongitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeRectLongitude(double)));
		connect(ui.clatitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeRectLatitude(double)));
		connect(ui.cheightSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeHeight(double)));
		connect(ui.rectLengthSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeRectLength(double)));
		connect(ui.rectWidthSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeRectWidth(double)));
		
		ui.label_12->hide();
		ui.cheightSpin->hide();
		ui.label_hei->hide();
	}

		void CRectStyleWidget::SlotChangeLineWidth( double dWidth)
		{
			m_opMark->SetLineWidth(dWidth);
		}

		void CRectStyleWidget::SlotChangeLineColor(  )
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

		void CRectStyleWidget::SlotChangeLineColorTransparant( double dTransparant)
		{
			if(m_opMark.valid())
			{
				osg::Vec4d vecColor = m_opMark->GetLineColor();
				vecColor.a() = dTransparant;
				m_opMark->SetLineColor(vecColor);
			}
		}

		void CRectStyleWidget::SlotChangeRectColor(  )
		{
			QColorDialog* pColorDlg = new QColorDialog(QColor(m_opMark->GetFillColor().r(), m_opMark->GetFillColor().g(), m_opMark->GetFillColor().b(), m_opMark->GetFillColor().a()));
			pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);
			
			if (pColorDlg->exec() == QDialog::Accepted)
			{
				QString strArcColor;
				strArcColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
					+ QString::number(pColorDlg->currentColor().blue()) + ");";
				ui.rectColorBtn->setStyleSheet(strArcColor);

				(dynamic_cast<FeExtNode::CExRectNode*>(m_opMark.get()))->SetFillColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, (int)ui.lineTransparantSpin->value()));
			}
		}

		void CRectStyleWidget::SlotChangeRectColorTransparant( double dTransparant)
		{
			FeExtNode::CExRectNode* pRect = dynamic_cast<FeExtNode::CExRectNode*>(m_opMark.get());
			if(pRect)
			{
				osg::Vec4d vecColor = pRect->GetFillColor();
				vecColor.a() = dTransparant;
				pRect->SetFillColor(vecColor);
			}
		}


		void CRectStyleWidget::SlotChangeRectLength( double dHeight)
		{
			m_opMark->SetHeight(dHeight);
		}

		void CRectStyleWidget::SlotChangeRectWidth( double dWidth)
		{
			m_opMark->SetWidth(dWidth);
		}

		void CRectStyleWidget::SlotChangeRectLongitude( double dLongitude)
		{
			osgEarth::GeoPoint npos = m_opMark->GetPosition();
			npos.x() = dLongitude;
			m_opMark->SetPosition(npos);
		}

		void CRectStyleWidget::SlotChangeRectLatitude( double dLatitude)
		{
			osgEarth::GeoPoint npos = m_opMark->GetPosition();
			npos.y() = dLatitude;
			m_opMark->SetPosition(npos);
		}

		void CRectStyleWidget::SlotChangeHeight( double dHeight)
		{
			osgEarth::GeoPoint npos = m_opMark->GetPosition();
			npos.z() = dHeight;
			m_opMark->SetPosition(npos);
		}

		void CRectStyleWidget::SlotSDKEventHappend( unsigned int eType)
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
