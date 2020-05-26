#include <mark/BillboardStyleWidget.h>

#include <mainWindow/FreeUtil.h>

#include <FeUtils/StrUtil.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/EventListener.h>
#include <QFileDialog>

namespace FreeViewer
{
	CBillboardStyleWidget::CBillboardStyleWidget(FeShell::CSystemService* pSystemService, FeExtNode::CExBillBoardNode *pMiliMarkNode, bool bCreate, QWidget *parent)
		:CBaseStyleWidget(parent)
		,m_opMark(pMiliMarkNode)
		,m_opSystemService(pSystemService)
		,m_bCreate(bCreate)
	{
		ui.setupUi(this);
		setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

		setWindowTitle(tr("BillboardMark"));

		connect(ui.pushButton,SIGNAL(clicked (bool)),this,SLOT(SlotRSPath(bool)));
		connect(ui.doubleSpinBox_Lon,SIGNAL(valueChanged(double)),this,SLOT(SlotLongChanged(double)));
		connect(ui.doubleSpinBox_Lat,SIGNAL(valueChanged(double)),this,SLOT(SlotLatChanged(double)));
		connect(ui.doubleSpinBox_Hight,SIGNAL(valueChanged(double)),this,SLOT(SlotHeightChanged(double)));
		connect(ui.doubleSpinBox_Scale,SIGNAL(valueChanged(double)),this,SLOT(SlotScaleChanged(double)));
		connect(ui.doubleSpinBox_Angle,SIGNAL(valueChanged(double)),this,SLOT(SlotAngleChanged(double)));
		connect(ui.spinBox_imageWidth, SIGNAL(valueChanged(int)), this, SLOT(SlotImageWidthChanged(int)));
		connect(ui.spinBox_imageHeight, SIGNAL(valueChanged(int)), this, SLOT(SlotImageHeightChanged(int)));
		connect(ui.Edit_RS, SIGNAL(textEdited(const QString&)), this, SLOT(SlotPathChanged(const QString&)));

		ui.groupBox_2->hide();

		InitTab();
	}

	void CBillboardStyleWidget::InitTab()
	{
		m_markOpt = *(m_opMark->GetOption());

		osgEarth::GeoPoint vecLLH;
// 		if(m_bCreate)
// 		{
// 			FeUtil::CFreeViewPoint freeViewPoint = m_opSystemService->GetViewPoint();
// 			vecLLH.x() = freeViewPoint.GetLon();
// 			vecLLH.y() = freeViewPoint.GetLat();
// 			FeUtil::DegreeLL2LLH(m_opSystemService->GetRenderContext(), vecLLH.vec3d());
// 		}
// 		else
		{
			vecLLH = m_opMark->GetPosition();
		}
		
		SetCenture(vecLLH);
		SetAngle(0.0);
		SetMarkImgPath(m_opMark->GetImagePath());
		SetImageSize(m_opMark->GetImageSize());
	}

	CBillboardStyleWidget::~CBillboardStyleWidget()
	{
	}

	void CBillboardStyleWidget::SetMarkImgPath(const std::string& strRS)
	{
		ui.Edit_RS->setText(strRS.c_str());
		m_opMark->SetImagePath(strRS);
	}

	std::string CBillboardStyleWidget::GetMarkImgPath() const
	{
		return ui.Edit_RS->text().toStdString();
	}

	void CBillboardStyleWidget::SetAngle(double dAngle)
	{
		ui.doubleSpinBox_Angle->setValue(dAngle);
	}

	double CBillboardStyleWidget::GetAngle() const
	{
		return ui.doubleSpinBox_Angle->value();
	}

	void CBillboardStyleWidget::SetScale(double dScale)
	{
		ui.doubleSpinBox_Scale->setValue(dScale);
	}

	double CBillboardStyleWidget::GetScale() const
	{
		return ui.doubleSpinBox_Scale->value();
	}

	void CBillboardStyleWidget::SetCenture(osgEarth::GeoPoint centure)
	{
		ui.doubleSpinBox_Lon->setValue(centure.x());
		ui.doubleSpinBox_Lat->setValue(centure.y());
		ui.doubleSpinBox_Hight->setValue(centure.z());
		if(m_bCreate)
		{
			osgEarth::GeoPoint point = m_opMark->GetPosition();
			point.x() = centure.x();
			point.y() = centure.y();
			point.z() = centure.z();

			m_opMark->SetPosition(point);
		}
	}

	bool CBillboardStyleWidget::Reject()
	{
		if(m_opMark.valid())
		{
			m_opMark->SetImagePath(m_markOpt.imagePath());
			m_opMark->SetPosition(m_markOpt.geoPoint());
			m_opMark->SetImageSize(m_markOpt.imageSize());
		}
		return true;
	}

	osg::Vec3 CBillboardStyleWidget::GetCenture() const
	{
		return osg::Vec3d(ui.doubleSpinBox_Lon->value(), ui.doubleSpinBox_Lat->value(), ui.doubleSpinBox_Hight->value());
	}

	void CBillboardStyleWidget::SlotRSPath(bool)
	{
		QString filePath;
		QString dirPath = FeFileReg->GetDataPath().c_str();
		dirPath += "/texture/mark";
#ifdef WIN32
		QFileDialog* pFileDialog = new QFileDialog();
		pFileDialog->setAttribute(Qt::WA_DeleteOnClose, true);
		filePath = pFileDialog->getOpenFileName(0, tr("open File"), dirPath, tr("Billboard Files(*.png);;All Files(*.*)"));
		if(!filePath.isNull())
		{
			ui.Edit_RS->setText(filePath);
			SlotPathChanged(filePath);
		}
#else
		QFileDialog fileDialog(0, tr("open File"), dirPath, tr("Billboard Files(*.png);;All Files(*.*)"));
		if (fileDialog.exec())
		{
			filePath = fileDialog.selectedFiles().first();
			ui.Edit_RS->setText(filePath);
			SlotPathChanged(filePath);
		}
#endif
	}

	void CBillboardStyleWidget::SlotPathChanged( const QString& str)
	{
		QString strPath = str;
		FormatPathString(strPath);

		if (m_opMark.valid())
		{
			m_opMark->SetImagePath(strPath.toStdString());
		}
	}

	void CBillboardStyleWidget::SlotLongChanged(double dLong)
	{
		osgEarth::GeoPoint point = m_opMark->GetPosition();
		point.x() = dLong;
		m_opMark->SetPosition(point);
	}

	void CBillboardStyleWidget::SlotLatChanged(double dLat)
	{
		osgEarth::GeoPoint point = m_opMark->GetPosition();
		point.y() = dLat;
		m_opMark->SetPosition(point);
	}

	void CBillboardStyleWidget::SlotHeightChanged(double dHeight)
	{
		osgEarth::GeoPoint point = m_opMark->GetPosition();
		point.z() = dHeight;
		m_opMark->SetPosition(point);
	}

	void CBillboardStyleWidget::SlotScaleChanged(double dScale)
	{
		//m_opMark->SetScale(osg::Vec3d(dScale, dScale, dScale));
	}

	void CBillboardStyleWidget::SlotAngleChanged(double dAngle)
	{
		//m_opMark->(dAngle);
	}

	void CBillboardStyleWidget::SetImageSize( osg::Vec2 vecImageSize )
	{
		ui.spinBox_imageWidth->setValue(vecImageSize.x());
		ui.spinBox_imageHeight->setValue(vecImageSize.y());
	}

	osg::Vec2 CBillboardStyleWidget::GetImageSize()
	{
		return osg::Vec2(ui.spinBox_imageWidth->value(), ui.spinBox_imageHeight->value());
	}

	void CBillboardStyleWidget::SlotImageWidthChanged( int nWidht )
	{
		osg::Vec2 vecImageSize = m_opMark->GetImageSize();
		vecImageSize.x() = nWidht;
		m_opMark->SetImageSize(vecImageSize);
	}

	void CBillboardStyleWidget::SlotImageHeightChanged( int nHeight )
	{
		osg::Vec2 vecImageSize = m_opMark->GetImageSize();
		vecImageSize.y() = nHeight;
		m_opMark->SetImageSize(vecImageSize);
	}

	void CBillboardStyleWidget::SlotSDKEventHappend( unsigned int eType)
	{
		if(!m_opMark.valid()) return;

		if(FeUtil::E_EVENT_TYPE_OBJECT_POSITION_CHANGED == eType)
		{
			SetCenture(m_opMark->GetPosition());
		}
	}

}

