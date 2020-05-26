#include <mark/PositionStyleWidget.h>

#include <mark/Common.h>
#include <mainWindow/FreeUtil.h>

#include <FeExtNode/ExPlaceNode.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/EventListener.h>

#include <QFileDialog>

namespace FreeViewer
{
	CPositionStyleWidget::CPositionStyleWidget(FeExtNode::CExternNode* opMark, QWidget *parent) 
		: CBaseStyleWidget(parent)
		,m_opMark(opMark)
	{
		ui.setupUi(this);
		setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
		initWidget();
	}


	CPositionStyleWidget::~CPositionStyleWidget(void)
	{
	}

	bool CPositionStyleWidget::Accept()
	{
		return true;
	}

	bool CPositionStyleWidget::Reject()
	{
		FeExtNode::CExPlaceNode* pPlaceMark = dynamic_cast<FeExtNode::CExPlaceNode*>(m_opMark.get());
		if(pPlaceMark)
		{
			pPlaceMark->SetTextSize(m_placeOpt.TextSize());
			pPlaceMark->SetImagePath(m_placeOpt.imagePath());
		}
		return true;
	}

	void CPositionStyleWidget::initWidget()
	{
		setWindowTitle(tr("Position"));

		FeExtNode::CExPlaceNode* pPlaceMark = dynamic_cast<FeExtNode::CExPlaceNode*>(m_opMark.get());
		if (pPlaceMark)
		{
			m_placeOpt = *pPlaceMark->GetOption();
			
			ui.dSpinBox_longitude->setValue(pPlaceMark->GetPosition().x());
			ui.dSpinBox_latitude->setValue(pPlaceMark->GetPosition().y());
			ui.dSpinBox_height->setValue(pPlaceMark->GetPosition().z());
			ui.Spin_FontSize->setValue(pPlaceMark->GetTextSize());
			ui.Edit_Pic_Path_2->setText(QString::fromStdString(pPlaceMark->GetImagePath()));
			connect(ui.pushButton_2, SIGNAL(clicked (bool)), this, SLOT(slot_pathBtnClick()));
			connect(ui.Spin_FontSize, SIGNAL(valueChanged(double)), this, SLOT(slot_fontSizeChanged(double)));
			connect(ui.Edit_Pic_Path_2, SIGNAL(textEdited(const QString&)), this, SLOT(SlotPathChanged(const QString&)));
			connect(ui.dSpinBox_longitude,SIGNAL(valueChanged(double)),this,SLOT(SlotLongChanged(double)));
			connect(ui.dSpinBox_latitude,SIGNAL(valueChanged(double)),this,SLOT(SlotLatChanged(double)));
			connect(ui.dSpinBox_height,SIGNAL(valueChanged(double)),this,SLOT(SlotHeightChanged(double)));
		}	

		ui.label_4->hide();
		ui.dSpinBox_height->hide();
		ui.label_hei->hide();
	}

	void CPositionStyleWidget::slot_pathBtnClick()
	{
		QString filePath;
		QString dirPath = FeFileReg->GetDataPath().c_str();
		dirPath += "/texture/mark";
#ifdef WIN32
		QFileDialog* pFileDialog = new QFileDialog();
		pFileDialog->setAttribute(Qt::WA_DeleteOnClose, true);
		filePath = pFileDialog->getOpenFileName(0, tr("open File"), dirPath, tr("Position Files(*.png);;All files (*.*)"));
		if(!filePath.isNull())
		{
			ui.Edit_Pic_Path_2->setText(filePath);
			SlotPathChanged(filePath);
		}
#else
		QFileDialog fileDialog(0, tr("open File"), dirPath, tr("Position Files(*.png);;All files (*.*)"));
		if (fileDialog.exec())
		{
			filePath = fileDialog.selectedFiles().first();
			ui.Edit_Pic_Path_2->setText(filePath);
			SlotPathChanged(filePath);
		}
#endif
	}

	void CPositionStyleWidget::slot_fontSizeChanged( double tsize )
	{
		FeExtNode::CExPlaceNode* pPlaceMark = dynamic_cast<FeExtNode::CExPlaceNode*>(m_opMark.get());
		pPlaceMark->SetTextSize(tsize);
	}

	void CPositionStyleWidget::SlotPathChanged( const QString& str)
	{
		QString strPath = str;
		FormatPathString(strPath);

		FeExtNode::CExPlaceNode* pPlaceMark = dynamic_cast<FeExtNode::CExPlaceNode*>(m_opMark.get());
		if (pPlaceMark)
		{
			pPlaceMark->SetImagePath(strPath.toStdString());
		}
	}

	void CPositionStyleWidget::SlotSDKEventHappend( unsigned int eType)
	{
		if(FeUtil::E_EVENT_TYPE_OBJECT_POSITION_CHANGED == eType)
		{
			FeExtNode::CExPlaceNode* pPlaceMark = dynamic_cast<FeExtNode::CExPlaceNode*>(m_opMark.get());
			if (pPlaceMark)
			{
				ui.dSpinBox_longitude->setValue(pPlaceMark->GetPosition().x());
				ui.dSpinBox_latitude->setValue(pPlaceMark->GetPosition().y());
				ui.dSpinBox_height->setValue(pPlaceMark->GetPosition().z());
			}
		}
	}

	void CPositionStyleWidget::SlotLongChanged( double dLong )
	{
		FeExtNode::CExPlaceNode* pPlaceMark = dynamic_cast<FeExtNode::CExPlaceNode*>(m_opMark.get());
		osgEarth::GeoPoint npos = pPlaceMark->GetPosition();
		npos.x() = dLong;
		pPlaceMark->SetPosition(npos);
	}

	void CPositionStyleWidget::SlotLatChanged( double dLat )
	{
		FeExtNode::CExPlaceNode* pPlaceMark = dynamic_cast<FeExtNode::CExPlaceNode*>(m_opMark.get());
		osgEarth::GeoPoint npos = pPlaceMark->GetPosition();
		npos.y() = dLat;
		pPlaceMark->SetPosition(npos);
	}

	void CPositionStyleWidget::SlotHeightChanged( double dHeight )
	{
		FeExtNode::CExPlaceNode* pPlaceMark = dynamic_cast<FeExtNode::CExPlaceNode*>(m_opMark.get());
		osgEarth::GeoPoint npos = pPlaceMark->GetPosition();
		npos.z() = dHeight;
		pPlaceMark->SetPosition(npos);
	}

}