#include <mark/OverLayStyleWidget.h>

#include <mainWindow/FreeUtil.h>

#include <FeUtils/StrUtil.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeExtNode/ExOverLayNode.h>
#include <FeUtils/EventListener.h>
#include <QFileDialog>


namespace FreeViewer
{
	COverLayStyleWidget::COverLayStyleWidget(FeShell::CSystemService* pSystemService, FeExtNode::CExOverLayNode *pMiliMarkNode, bool bCreate, QWidget *parent)
		:CBaseStyleWidget(parent)
		,m_opMark(pMiliMarkNode)
		,m_opSystemService(pSystemService)
		,m_bCreate(bCreate)
	{
		ui.setupUi(this);
		setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

		setWindowTitle(tr("OverLayMark"));

		InitTab();

		ui.label_4->hide();
		ui.doubleSpinBox_Hight->hide();
		ui.label_hei->hide();

		connect(ui.pushButton,SIGNAL(clicked (bool)),this,SLOT(SlotRSPath(bool)));

		connect(ui.doubleSpinBox_Lon,SIGNAL(valueChanged(double)),this,SLOT(SlotLongChanged(double)));
		connect(ui.doubleSpinBox_Lat,SIGNAL(valueChanged(double)),this,SLOT(SlotLatChanged(double)));
		connect(ui.doubleSpinBox_Scale,SIGNAL(valueChanged(double)),this,SLOT(SlotScaleChanged(double)));
		connect(ui.doubleSpinBox_Angle,SIGNAL(valueChanged(double)),this,SLOT(SlotAngleChanged(double)));
		connect(ui.doubleSpinBox_Alpha, SIGNAL(valueChanged(double)), this, SLOT(SlotAlphaChanged(double)));
		connect(ui.doubleSpinBox_imageWidth, SIGNAL(valueChanged(double)), this, SLOT(SlotImageWidthChanged(double)));
		connect(ui.doubleSpinBox_imageHeight, SIGNAL(valueChanged(double)), this, SLOT(SlotImageHeightChanged(double)));
		connect(ui.Edit_RS, SIGNAL(textEdited(const QString&)), this, SLOT(SlotPathChanged(const QString&)));
		
		ui.groupBox_2->hide();
	}

	void COverLayStyleWidget::InitTab()
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
		//SetAngle(m_opMark->GetPosture().z());
		//SetScale(m_opMark->GetScale().z());
		SetMarkImgPath(m_opMark->GetImagePath());
		//SetAlpha(m_opMark->GetColor().w());
		SetImageSize(m_opMark->GetImageSize());
	}

	COverLayStyleWidget::~COverLayStyleWidget()
	{
	}

	void COverLayStyleWidget::SetMarkImgPath(const std::string& strRS)
	{
		ui.Edit_RS->setText(strRS.c_str());
		m_opMark->SetImagePath(strRS.c_str());
	}

	std::string COverLayStyleWidget::GetMarkImgPath() const
	{
		return ui.Edit_RS->text().toStdString();
	}

	double COverLayStyleWidget::GetAlpha()
	{
		return ui.doubleSpinBox_Alpha->value();
	}

	void COverLayStyleWidget::SetAlpha(double dAlpha)
	{
		ui.doubleSpinBox_Alpha->setValue(dAlpha);
	}

	void COverLayStyleWidget::SetAngle(double dAngle)
	{
		ui.doubleSpinBox_Angle->setValue(dAngle);
	}

	double COverLayStyleWidget::GetAngle() const
	{
		return ui.doubleSpinBox_Angle->value();
	}

	void COverLayStyleWidget::SetScale(double dScale)
	{
		ui.doubleSpinBox_Scale->setValue(dScale);
	}

	double COverLayStyleWidget::GetScale() const
	{
		return ui.doubleSpinBox_Scale->value();
	}

	void COverLayStyleWidget::SetCenture(const osgEarth::GeoPoint& centure)
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

	bool COverLayStyleWidget::Reject()
	{
		if(m_opMark.valid())
		{
			m_opMark->SetImagePath(m_markOpt.imagePath());
			m_opMark->SetPosition(m_markOpt.geoPoint());
			m_opMark->SetImageSize(m_markOpt.imageSize());
		}
		return true;
	}

	osg::Vec3 COverLayStyleWidget::GetCenture() const
	{
		return osg::Vec3d(ui.doubleSpinBox_Lon->value(), ui.doubleSpinBox_Lat->value(), ui.doubleSpinBox_Hight->value());
	}

	void COverLayStyleWidget::SlotRSPath(bool)
	{
		QString filePath;
		QString dirPath = FeFileReg->GetDataPath().c_str();
		dirPath += "/texture/mark";
#ifdef WIN32
		QFileDialog* pFileDialog = new QFileDialog();
		pFileDialog->setAttribute(Qt::WA_DeleteOnClose, true);
		filePath = pFileDialog->getOpenFileName(0, tr("open File"), dirPath, tr("OverLay Files(*.png);;All Files(*.*)"));
		if(!filePath.isNull())
		{
			ui.Edit_RS->setText(filePath);
			SlotPathChanged(filePath);
		}
#else
		QFileDialog fileDialog(0, tr("open File"), dirPath, tr("OverLay Files(*.png);;All Files(*.*)"));
		if (fileDialog.exec())
		{
			filePath = fileDialog.selectedFiles().first();
			ui.Edit_RS->setText(filePath);
			SlotPathChanged(filePath);
		}
#endif
	}

	void COverLayStyleWidget::SlotPathChanged( const QString& str)
	{
		QString strPath = str;
		FormatPathString(strPath);

		if(m_opMark.valid())
		{
			m_opMark->SetImagePath(strPath.toStdString());
		}
	}

	void COverLayStyleWidget::SlotLongChanged(double dLong)
	{
		osgEarth::GeoPoint point = m_opMark->GetPosition();
		point.x() = dLong;
		m_opMark->SetPosition(point);
	}

	void COverLayStyleWidget::SlotLatChanged(double dLat)
	{
		osgEarth::GeoPoint point = m_opMark->GetPosition();
		point.y() = dLat;
		m_opMark->SetPosition(point);
	}

	void COverLayStyleWidget::SlotScaleChanged(double dScale)
	{
		//m_opMark->SetScale(osg::Vec3d(dScale, dScale, dScale));
	}

	void COverLayStyleWidget::SlotAngleChanged(double dAngle)
	{
		//m_opMark->SetPosture(osg::Vec3d(dAngle, dAngle, dAngle));
	}


	void COverLayStyleWidget::SlotAlphaChanged(double dAlpha)
	{
		//m_opMark->SetColor(osg::Vec4d(dAlpha, dAlpha, dAlpha, dAlpha));
	}

	void COverLayStyleWidget::SlotSDKEventHappend( unsigned int eType)
	{
		if(!m_opMark.valid()) return;

		if(FeUtil::E_EVENT_TYPE_OBJECT_POSITION_CHANGED == eType)
		{
			SetCenture(m_opMark->GetPosition());
		}
	}

	void COverLayStyleWidget::SetImageSize( osg::Vec2 vecImageSize )
	{
		ui.doubleSpinBox_imageWidth->setValue(vecImageSize.x());
		ui.doubleSpinBox_imageHeight->setValue(vecImageSize.y());
	}

	osg::Vec2 COverLayStyleWidget::GetImageSize()
	{
		return osg::Vec2(ui.doubleSpinBox_imageWidth->value(), ui.doubleSpinBox_imageHeight->value());
	}

	void COverLayStyleWidget::SlotImageWidthChanged( double nWidht )
	{
		osg::Vec2 vecImageSize = m_opMark->GetImageSize();
		vecImageSize.x() = nWidht;
		m_opMark->SetImageSize(vecImageSize);
	}

	void COverLayStyleWidget::SlotImageHeightChanged( double nHeight )
	{
		osg::Vec2 vecImageSize = m_opMark->GetImageSize();
		vecImageSize.y() = nHeight;
		m_opMark->SetImageSize(vecImageSize);
	}
}

