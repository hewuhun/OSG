#include <QColorDialog>
#include <FeExtNode/ExTextNode.h>
#include <FeUtils/EventListener.h>
#include <mark/TextStyleWidget.h>

namespace FreeViewer
{
	CTextStyleWidget::CTextStyleWidget(FeExtNode::CExTextNode* opMark, QWidget *parent) 
		: CBaseStyleWidget(parent)
		,m_opMark(opMark)
	{
		ui.setupUi(this);
		setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
		initWidget();
	}


	CTextStyleWidget::~CTextStyleWidget(void)
	{
	}

	void CTextStyleWidget::initWidget()
	{
		setWindowTitle(tr("Text"));

		FeExtNode::CExTextNode* pTextMark = dynamic_cast<FeExtNode::CExTextNode*>(m_opMark.get());
		if (pTextMark)
		{
			m_markOpt = *(pTextMark->GetOption());

			QString strTextColor;
			strTextColor = "background-color:rgb(" 
			+ QString::number(pTextMark->GetTextColor().r() * 255) + ","
			+ QString::number(pTextMark->GetTextColor().g() * 255) + ","
			+ QString::number(pTextMark->GetTextColor().b() * 255) + ");";
			ui.textColorBtn->setStyleSheet(strTextColor);
			ui.textSizeSpin->setValue(pTextMark->GetTextSize());
			osgEarth::GeoPoint vpos = m_opMark->GetPosition();
			ui.clongitudeSpin->setValue(vpos.x());
			ui.clantitudeSpin->setValue(vpos.y());
			ui.cheightSpin->setValue(vpos.z());
			connect(ui.textColorBtn,SIGNAL(clicked()),this,SLOT(SlotTextColor()));
			connect(ui.textSizeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotTextSize(double)));
			
			connect(ui.clongitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeSectorLongitude(double)));
			connect(ui.clantitudeSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeSectorLatitude(double)));
			connect(ui.cheightSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeHeight(double)));

			ui.Lat->hide();
			ui.cheightSpin->hide();
			ui.label_hei->hide();
		}		
	}

	void CTextStyleWidget::SlotTextColor()
	{
		FeExtNode::CExTextNode* pTextMark = dynamic_cast<FeExtNode::CExTextNode*>(m_opMark.get());
		QColorDialog* pColorDlg = new QColorDialog(QColor(pTextMark->GetTextColor().r(), pTextMark->GetTextColor().g(), pTextMark->GetTextColor().b(), pTextMark->GetTextColor().a()));
		if (pColorDlg->exec() == QDialog::Accepted)
		{
			QString strTextColor;
			strTextColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
				+ QString::number(pColorDlg->currentColor().blue()) + ");";
			ui.textColorBtn->setStyleSheet(strTextColor);
			(dynamic_cast<FeExtNode::CExTextNode*>(m_opMark.get()))->SetTextColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, 1.0));
		}
		
	}

	void CTextStyleWidget::SlotTextSize( double tsize)
	{
		FeExtNode::CExTextNode* pTextMark = dynamic_cast<FeExtNode::CExTextNode*>(m_opMark.get());
		pTextMark->SetTextSize(tsize);
	}

	void CTextStyleWidget::SlotChangeSectorLongitude( double dLongitude)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.x() = dLongitude;
		m_opMark->SetPosition(npos);
	}

	void CTextStyleWidget::SlotChangeSectorLatitude( double dLatitude)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();
		npos.y() = dLatitude;
		m_opMark->SetPosition(npos);
	}

	void CTextStyleWidget::SlotChangeHeight( double dHeight)
	{
		osgEarth::GeoPoint npos = m_opMark->GetPosition();  
		npos.z() = dHeight;
		m_opMark->SetPosition(npos);
	}

	bool CTextStyleWidget::Reject()
	{
		if(m_opMark.valid())
		{
			m_opMark->SetPosition(m_markOpt.geoPoint());
			m_opMark->SetTextColor(m_markOpt.TextColor());
			m_opMark->SetTextSize(m_markOpt.TextSize());
		}
		return true;
	}

	void CTextStyleWidget::SlotSDKEventHappend( unsigned int eType)
	{
		if(!m_opMark.valid()) return;

		if(FeUtil::E_EVENT_TYPE_OBJECT_POSITION_CHANGED == eType)
		{
			osgEarth::GeoPoint vpos = m_opMark->GetPosition();
			ui.clongitudeSpin->setValue(vpos.x());
			ui.clantitudeSpin->setValue(vpos.y());
			ui.cheightSpin->setValue(vpos.z());
		}
	}

}