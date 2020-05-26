#include <QColorDialog>
#include <FeExtNode/ExPlaceNode.h>
#include <FeExtNode/ExPolygonNode.h>
#include <FeUtils/EventListener.h>
#include <mark/Common.h>
#include <mark/LineStyleWidget.h>

namespace FreeViewer
{
	CLineStyleWidget::CLineStyleWidget( FeExtNode::CExFeatureNode* opMark, QWidget *parent /*= 0*/ )
		: CBaseStyleWidget(parent)
		,m_opMark(opMark)
	{
		ui.setupUi(this);
		setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

		setWindowTitle(tr("Style"));

		initWidget();
	}

	void CLineStyleWidget::initWidget()
	{
		m_featureOpt = *m_opMark->GetOption();

		QString strLineColor;
		strLineColor = "background-color:rgb(" 
			+ QString::number(m_opMark->GetLineColor().r() * 255) + ","
			+ QString::number(m_opMark->GetLineColor().g() * 255) + ","
			+ QString::number(m_opMark->GetLineColor().b() * 255) + ");";
		ui.lineColorBtn->setStyleSheet(strLineColor);

		ui.lineWidthSpin->setValue(m_opMark->GetLineWidth());
		ui.lineTransparantSpin->setValue(m_opMark->GetLineColor().a());


		FeExtNode::CExPolygonNode* pPolygon = dynamic_cast<FeExtNode::CExPolygonNode*>(m_opMark.get());
		if (pPolygon)
		{
			ui.polygonTransparantSpin->setValue(pPolygon->GetFillColor().a());

			QString strPolygonColor;
			
			strPolygonColor = "background-color:rgb(" + QString::number(pPolygon->GetFillColor().r() * 255) + "," + QString::number(pPolygon->GetFillColor().g() * 255) + ","
				+ QString::number(pPolygon->GetFillColor().b() * 255) + ");";
			ui.polygonColorBtn->setStyleSheet(strPolygonColor);

		}
		
		ui.heightSpin->setValue(0.0);

		connect(ui.lineTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineColorTransparant(double)));
		connect(ui.lineWidthSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeLineWidth(double)));
		connect(ui.polygonTransparantSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangePolygonColorTransparant(double)));

		connect(ui.lineColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeLineColor()));
		connect(ui.polygonColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangePolygonColor()));

		connect(ui.heightSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotChangeHeight(double)));

		//
		ui.label_7->hide();
		ui.heightSpin->hide();
		ui.groupBox->hide();
	}

	void CLineStyleWidget::setPolygonStyleVisible( bool bVisible )
	{
		ui.polygonStyleGroup->setVisible(bVisible);
	}

	bool CLineStyleWidget::Accept()
	{
		return true;
	}

	bool CLineStyleWidget::Reject()
	{
		if(m_opMark.valid())
		{
			m_opMark->SetVisible(m_featureOpt.visible());
			m_opMark->SetLineWidth(m_featureOpt.lineWidth());
			m_opMark->SetLineColor(m_featureOpt.lineColor());
			m_opMark->SetFillColor(m_featureOpt.fillColor());
		}
		return true;
	}

	void CLineStyleWidget::SlotChangeLineWidth( double dWidth)
	{
		m_opMark->SetLineWidth(dWidth);
	}

	void CLineStyleWidget::SlotChangeLineColor(  )
	{
		//disconnect(ui.lineColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeLineColor()));


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
		//connect(ui.lineColorBtn,SIGNAL(clicked()),this,SLOT(SlotChangeLineColor()));


	}

	void CLineStyleWidget::SlotChangeLineColorTransparant( double dTransparant)
	{
		if(m_opMark.valid())
		{
			osg::Vec4d vecColor = m_opMark->GetLineColor();
			vecColor.a() = dTransparant;
			m_opMark->SetLineColor(vecColor);
		}

	}

	void CLineStyleWidget::SlotChangePolygonColor(  )
	{
		QColorDialog* pColorDlg = new QColorDialog(QColor(m_opMark->GetLineColor().r(), m_opMark->GetLineColor().g(), m_opMark->GetLineColor().b(), m_opMark->GetLineColor().a()));
		pColorDlg->setWindowFlags(Qt::WindowStaysOnTopHint);

		if (pColorDlg->exec() == QDialog::Accepted)
		{
			QString strPolygonColor;
			strPolygonColor = "background-color:rgb(" + QString::number(pColorDlg->currentColor().red()) + "," + QString::number(pColorDlg->currentColor().green()) + ","
				+ QString::number(pColorDlg->currentColor().blue()) + ");";
			ui.polygonColorBtn->setStyleSheet(strPolygonColor);

			//m_opMark->SetColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, (int)ui.lineTransparantSpin->value()));
			(dynamic_cast<FeExtNode::CExFeatureNode*>(m_opMark.get()))->SetFillColor(osg::Vec4d(pColorDlg->currentColor().red() / 255.0, pColorDlg->currentColor().green() / 255.0, pColorDlg->currentColor().blue() / 255.0, ui.polygonTransparantSpin->value()));
		}
		//delete pColorDlg;
	}

	void CLineStyleWidget::SlotChangePolygonColorTransparant( double dTransparant)
	{
		FeExtNode::CExFeatureNode* pPolygon = dynamic_cast<FeExtNode::CExFeatureNode*>(m_opMark.get());
		if(pPolygon)
		{
			osg::Vec4d vecColor = pPolygon->GetFillColor();
			vecColor.a() = dTransparant;
			pPolygon->SetFillColor(vecColor);
		}

	}

	void CLineStyleWidget::SlotChangeHeight( double dHeight)
	{
		//m_opMark->SetHeight(dHeight);
	}

	void CLineStyleWidget::SlotSDKEventHappend( unsigned int eType)
	{
	}
}
