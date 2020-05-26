#include <mark/ModelStyleWidget.h>

#include <mainWindow/FreeUtil.h>

#include <FeUtils/StrUtil.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/EventListener.h>
#include <FeServiceProvider/CurlHelper.h>

#include <QFileDialog>

namespace FreeViewer
{
	CModelStyleWidget::CModelStyleWidget(FeShell::CSystemService* pSystemService, FeExtNode::CExLodModelNode *pModelNode, bool bCreate, QWidget *parent)
		:CBaseStyleWidget(parent)
		,m_pModelNode(pModelNode)
		,m_opSystemService(pSystemService)
		,m_bCreate(bCreate)
		,m_bServicePathChanged(false)
	{
		ui.setupUi(this);

		setWindowTitle(tr("Model"));

		connect(ui.simpleButton,SIGNAL(clicked (bool)),this,SLOT(SlotSimplePath(bool)));
		connect(ui.comboBox_service, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotServicePath(int)));

		connect(ui.doubleSpinBox_Lon,SIGNAL(valueChanged(double)),this,SLOT(SlotLongChanged(double)));
		connect(ui.doubleSpinBox_Lat,SIGNAL(valueChanged(double)),this,SLOT(SlotLatChanged(double)));
		connect(ui.doubleSpinBox_Hight,SIGNAL(valueChanged(double)),this,SLOT(SlotHeightChanged(double)));
		connect(ui.doubleSpinBox_Scale,SIGNAL(valueChanged(double)),this,SLOT(SlotScaleChanged(double)));
		connect(ui.xAngleSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotXAngleChanged(double)));
		connect(ui.yAngleSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotYAngleChanged(double)));
		connect(ui.zAngleSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotZAngleChanged(double)));

		connect(ui.simpleLowSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotSimpleLow(double)));
		connect(ui.simpleHighSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotSimpleHigh(double)));
		connect(ui.serviceLowSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotServiceLow(double)));
		connect(ui.serviceHighSpin,SIGNAL(valueChanged(double)),this,SLOT(SlotServiceHigh(double)));

		connect(ui.simplePath, SIGNAL(textEdited(const QString&)), this, SLOT(SlotSimplePathChanged(const QString&)));
		connect(ui.pushButton_link, SIGNAL(clicked()), this, SLOT(SlotServicePathLinked()));
		connect(ui.servicePath, SIGNAL(textEdited(const QString&)), this, SLOT(SlotServicePathChanged(const QString&)));
		//将servicePath 提升为FreeLineEdit
		connect(ui.servicePath, SIGNAL(clicked()), this, SLOT(SlotUrlClicked()));
		connect(ui.servicePath, SIGNAL(editingFinished()), this, SLOT(SlotUrlEditFinished()));

		//隐藏服务模型控件
		ui.groupBox_service->hide();
		ui.radioButton_local->setChecked(true);
		connect(ui.radioButton_local, SIGNAL(clicked()), this, SLOT(SLotLocalRadioClicked()));
		connect(ui.radioButton_service, SIGNAL(clicked()), this, SLOT(SLotServiceRadioClicked()));

		InitService();
		InitTab();
	}

	void CModelStyleWidget::InitTab()
	{	
			m_markOpt = *(m_pModelNode->GetOption());
			m_mapModelInfo[FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE] = ModelInfo(
				m_pModelNode->GetLodModelNodeInfo(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE));

		if (FeExtNode::CExLodModelNodeOption::E_MODEL_TYPE_SERVICE ==  m_pModelNode->GetModelType())
		{
			ui.comboBox_service->setCurrentIndex(ui.comboBox_service->count()-1);
			ui.servicePath->setText(m_pModelNode->GetLodModelNodeInfo(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE).nodePath.c_str());
			m_pModelNode->GetLodModelRange(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE,dSimpleLow,dSimpleHigh);
			ui.servicePath->setStyleSheet("color : white");
			m_bServicePathChanged = true;
			ui.serviceLowSpin->setValue(dSimpleLow);
			ui.serviceHighSpin->setValue(dSimpleHigh);
			ui.radioButton_service->setChecked(true);
			SLotServiceRadioClicked();
		}
		else
		{
			ui.simplePath->setText(m_pModelNode->GetLodModelNodeInfo(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE).nodePath.c_str());
			m_pModelNode->GetLodModelRange(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE,dSimpleLow,dSimpleHigh);
			ui.simpleLowSpin->setValue(dSimpleLow);
			ui.simpleHighSpin->setValue(dSimpleHigh);
			ui.serviceLowSpin->setValue(dSimpleLow);
			ui.serviceHighSpin->setValue(dSimpleHigh);
			ui.radioButton_local->setChecked(true);
			SLotLocalRadioClicked();

			if (m_bCreate)
			{
				m_pModelNode->SetModelType(FeExtNode::CExLodModelNodeOption::E_MODEL_TYPE_LOCAL);
			}
		}

		SetCenture(m_pModelNode->GetPositionLLH());
		SetScale(m_pModelNode->GetScale().x());
		ui.xAngleSpin->setValue((m_pModelNode->GetRotate().x()));
		ui.yAngleSpin->setValue((m_pModelNode->GetRotate().y()));
		ui.zAngleSpin->setValue((m_pModelNode->GetRotate().z()));
	}

	CModelStyleWidget::~CModelStyleWidget()
	{
	}

	bool CModelStyleWidget::Reject()
	{
		if(m_pModelNode)
		{
			ModelInfo& modelInfo = m_mapModelInfo[FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE];
			m_pModelNode->SetLodModelNode(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE, 
				modelInfo.lodNodeInfo.nodePath, modelInfo.lodNodeOpt.minRange(), modelInfo.lodNodeOpt.maxRange());

			m_pModelNode->SetPositionByLLH(m_markOpt.PositionLLHDegree());
			m_pModelNode->SetScale(m_markOpt.ScaleDegree());
			m_pModelNode->SetRotate(m_markOpt.PostureDegree());
			m_pModelNode->SetModelType(m_markOpt.ModelType());
		}
		return true;
	}

	void CModelStyleWidget::SetScale( double dScale )
	{
		ui.doubleSpinBox_Scale->setValue(dScale);
	}

	double CModelStyleWidget::GetScale() const
	{
		return ui.doubleSpinBox_Scale->value();
	}

	void CModelStyleWidget::SetCenture( const osg::Vec3& centure )
	{
		ui.doubleSpinBox_Lon->setValue(centure.x());
		ui.doubleSpinBox_Lat->setValue(centure.y());
		ui.doubleSpinBox_Hight->setValue(centure.z());
		if(m_bCreate)
		{
			m_pModelNode->SetPositionByLLH(centure);
		}
	}

	osg::Vec3 CModelStyleWidget::GetCenture() const
	{
		return osg::Vec3d(ui.doubleSpinBox_Lon->value(), ui.doubleSpinBox_Lat->value(), ui.doubleSpinBox_Hight->value());
	}

	void CModelStyleWidget::SlotSimplePathChanged(const QString& str)
	{
		QString strPath = str;
		FormatPathString(strPath);

		if(m_pModelNode)
		{
			if(m_pModelNode->SetLodModelNode(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE,
				strPath.toStdString(),ui.simpleLowSpin->value(),ui.simpleHighSpin->value()))
			{
				m_pModelNode->SetModelType(FeExtNode::CExLodModelNodeOption::E_MODEL_TYPE_LOCAL);
				ui.servicePath->setText("");
			}
		}
	}
	
	void CModelStyleWidget::SlotSDKEventHappend( unsigned int eType)
	{
		if(!m_pModelNode) return;

		if(FeUtil::E_EVENT_TYPE_OBJECT_POSITION_CHANGED == eType)
		{
 			SetCenture(m_pModelNode->GetPositionLLH());
 		}
		else if(FeUtil::E_EVENT_TYPE_OBJECT_POSTURE_CHANGED == eType)
		{
			osg::Vec3d vecRotate = m_pModelNode->GetRotate();
			ui.xAngleSpin->setValue(vecRotate.x());
			ui.yAngleSpin->setValue(vecRotate.y());
			ui.zAngleSpin->setValue(vecRotate.z());
		}
		else if(FeUtil::E_EVENT_TYPE_OBJECT_SIZE_CHANGED == eType)
		{
			SetScale(m_pModelNode->GetScale().x());
		}
	}

	void CModelStyleWidget::SlotSimplePath( bool )
	{
		QString filePath;
		QString dirPath = FeFileReg->GetDataPath().c_str();
		dirPath += "/model";
#ifdef WIN32
		QFileDialog* pFileDialog = new QFileDialog();
		pFileDialog->setAttribute(Qt::WA_DeleteOnClose, true);
		filePath = pFileDialog->getOpenFileName(0, tr("open File"), dirPath, tr("Model Files(*.ive);;All files (*.*)"));
		if(!filePath.isNull())
		{
			ui.simplePath->setText(filePath);
			SlotSimplePathChanged(filePath);
		}
#else
		QFileDialog fileDialog(0, tr("open File"), dirPath, tr("Model Files(*.ive);;All files (*.*)"));
		if (fileDialog.exec())
		{
			filePath = fileDialog.selectedFiles().first();
			ui.simplePath->setText(filePath);
			SlotSimplePathChanged(filePath);
		}
#endif
	}

	void CModelStyleWidget::SlotServicePath( int index)
	{
		QString strUrl = ui.comboBox_service->itemData(index).toString();
		ui.servicePath->setText(strUrl);
		if (ui.comboBox_service->count()-1 == index)
		{
			ui.servicePath->setReadOnly(false);
			m_bServicePathChanged = false;
			ui.servicePath->clear();
			ui.servicePath->setText(tr("please input the model path"));
			ui.servicePath->setStyleSheet("color : gray");
		}
		else
		{
			ui.servicePath->setReadOnly(true);
			ui.servicePath->setStyleSheet("color : white");
			m_bServicePathChanged = true;
		}
	}

	void CModelStyleWidget::SlotLongChanged( double dLong)
	{
		osg::Vec3d vecLLH = m_pModelNode->GetPositionLLH();
		vecLLH.x() = dLong;
		m_pModelNode->SetPositionByLLH(vecLLH);
	}

	void CModelStyleWidget::SlotLatChanged( double dLat)
	{
		osg::Vec3d vecLLH = m_pModelNode->GetPositionLLH();
		vecLLH.y() = dLat;
		m_pModelNode->SetPositionByLLH(vecLLH);
	}

	void CModelStyleWidget::SlotHeightChanged( double dHeight)
	{
		osg::Vec3d vecLLH = m_pModelNode->GetPositionLLH();
		vecLLH.z() = dHeight;
		m_pModelNode->SetPositionByLLH(vecLLH);
	}

	void CModelStyleWidget::SlotScaleChanged( double dScale )
	{
		m_pModelNode->SetScale(osg::Vec3d(dScale, dScale, dScale));
	}

	void CModelStyleWidget::SlotZAngleChanged( double dAngle)
	{
		m_pModelNode->SetRotate(osg::Vec3d(m_pModelNode->GetRotate().x(), m_pModelNode->GetRotate().y(), dAngle));
	}

	void CModelStyleWidget::SlotXAngleChanged( double dAngle)
	{
		m_pModelNode->SetRotate(osg::Vec3d(dAngle, m_pModelNode->GetRotate().y(), m_pModelNode->GetRotate().z()));
	}

	void CModelStyleWidget::SlotYAngleChanged( double dAngle)
	{
		m_pModelNode->SetRotate(osg::Vec3d( m_pModelNode->GetRotate().x(),dAngle, m_pModelNode->GetRotate().z()));
	}

	void CModelStyleWidget::SlotSimpleLow( double dLow)
	{
		dSimpleLow = dLow;
		m_pModelNode->SetLodModelRange(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE,dSimpleLow,dSimpleHigh);
	}

	void CModelStyleWidget::SlotSimpleHigh( double dHigh)
	{
		dSimpleHigh = dHigh;
		m_pModelNode->SetLodModelRange(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE,dSimpleLow,dHigh);
	}


	void CModelStyleWidget::SlotServiceLow( double dLow)
	{
		dServiceLow = dLow;
		m_pModelNode->SetLodModelRange(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE, dServiceLow, dServiceHigh);
	}

	void CModelStyleWidget::SlotServiceHigh( double dHigh)
	{
		dServiceHigh = dHigh;
		m_pModelNode->SetLodModelRange(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE, dServiceLow, dServiceHigh);
	}

	void CModelStyleWidget::SLotLocalRadioClicked()
	{
		ui.groupBox_simple->setVisible(true);
		ui.groupBox_service->setVisible(false);
		ui.label_more->setVisible(false);
		ui.label_website->setVisible(false);
	}

	void CModelStyleWidget::SLotServiceRadioClicked()
	{
		ui.groupBox_simple->setVisible(false);
		ui.groupBox_service->setVisible(true);
		ui.label_more->setVisible(true);
		ui.label_website->setVisible(true);
	}

	void CModelStyleWidget::InitService()
	{
		ui.comboBox_service->blockSignals(true);
		ui.comboBox_service->addItem("飞机", "http://39.104.75.125:8090/FreeServer/model/ive/B737.ive");
		ui.comboBox_service->addItem("坦克", "http://39.104.75.125:8090/FreeServer/model/ive/Tank.ive");
		ui.comboBox_service->addItem("雷达", "http://39.104.75.125:8090/FreeServer/model/ive/Radar.ive");
		ui.comboBox_service->addItem("发射车", "http://39.104.75.125:8090/FreeServer/model/ive/MissileCar.ive");
		ui.comboBox_service->addItem(tr("custom")) ;
		ui.comboBox_service->blockSignals(false);
		ui.comboBox_service->setCurrentIndex(ui.comboBox_service->count()-1);
		ui.label_more->setText(tr("more service model please visite :"));
		ui.label_more->setStyleSheet("color: gray;");
	}

	void CModelStyleWidget::SlotServicePathLinked()
	{
		QString strPath = ui.servicePath->text();

		if(m_pModelNode)
		{
			if(m_pModelNode->SetLodModelNode(FeExtNode::CExLodModelNode::LOD_MODEL_SIMPLE,
				strPath.toStdString(),ui.serviceLowSpin->value(),ui.serviceHighSpin->value()))
			{
				m_pModelNode->SetModelType(FeExtNode::CExLodModelNodeOption::E_MODEL_TYPE_SERVICE);
				ui.simplePath->setText("");
			}
		}
	}

	void CModelStyleWidget::SlotServicePathChanged(const QString& str)
	{
		if (!m_bServicePathChanged)
		{
			ui.servicePath->setStyleSheet("color : white");
			m_bServicePathChanged = true;
		}
	}

	void CModelStyleWidget::SlotUrlEditFinished()
	{
		if ("" == ui.servicePath->text())
		{
			ui.servicePath->setText(tr("please input the model path"));
			ui.servicePath->setStyleSheet("color : gray");
			m_bServicePathChanged = false;
		}
	}

	void CModelStyleWidget::SlotUrlClicked()
	{
		if (!m_bServicePathChanged)
		{
			ui.servicePath->clear();
			ui.servicePath->setStyleSheet("color : white");
			m_bServicePathChanged = false;
		}
	}

}

