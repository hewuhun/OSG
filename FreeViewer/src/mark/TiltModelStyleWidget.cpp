#include <mark/TiltModelStyleWidget.h>

#include <mainWindow/FreeUtil.h>

#include <FeUtils/StrUtil.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <QFileDialog>

namespace FreeViewer
{
	CTiltModelStyleWidget::CTiltModelStyleWidget(FeShell::CSystemService* pSystemService, FeExtNode::CExTiltModelNode *pTiltModelNode, bool bCreate, QWidget *parent)
		:CBaseStyleWidget(parent)
		,m_pTiltModelNode(pTiltModelNode)
		,m_opSystemService(pSystemService)
		,m_bCreate(bCreate)
		,m_bServicePathChanged(false)
		,m_bInit(true)
	{
		ui.setupUi(this);

		m_markOpt = *(m_pTiltModelNode->GetOption());

		setWindowTitle(tr("TiltModel"));
		ui.radioButton_filePath->setChecked(true);
		ui.lineEdit_path->setReadOnly(true);
		connect(ui.doubleSpinBox_Lon,SIGNAL(valueChanged(double)),this,SLOT(SlotLonChanged(double)));
		connect(ui.doubleSpinBox_Lat,SIGNAL(valueChanged(double)),this,SLOT(SlotLatChanged(double)));
		connect(ui.doubleSpinBox_Hight,SIGNAL(valueChanged(double)),this,SLOT(SlotHeightChanged(double)));
		connect(ui.doubleSpinBox_Scale,SIGNAL(valueChanged(double)),this,SLOT(SlotScaleChanged(double)));
		connect(ui.doubleSpinBox_Angle,SIGNAL(valueChanged(double)),this,SLOT(SlotAngleChanged(double)));
		connect(ui.btn_browse,SIGNAL(clicked(bool)),this,SLOT(SlotBorwseBtnClicked(bool)));
		connect(ui.comboBox_service, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotServicePath(int)));
		connect(ui.pushButton_link, SIGNAL(clicked()), this, SLOT(SlotServicePathLinked()));

		connect(ui.radioButton_dataPath, SIGNAL(clicked()), this, SLOT(SlotLocalRadioClikced()));
		connect(ui.radioButton_filePath, SIGNAL(clicked()), this, SLOT(SlotLocalRadioClikced()));
		connect(ui.radioButton_service, SIGNAL(clicked()), this, SLOT(SlotServiceRadioClicked()));

		connect(ui.lineEdit_service_path, SIGNAL(textEdited(const QString&)), this, SLOT(SlotServicePathChanged(const QString&)));
		//将lineEdit_service_path 提升为FreeLineEdit
		connect(ui.lineEdit_service_path, SIGNAL(clicked()), this, SLOT(SlotUrlClicked()));
		connect(ui.lineEdit_service_path, SIGNAL(editingFinished()), this, SLOT(SlotUrlEditFinished()));

		//初始化显示本地模型界面
		SlotLocalRadioClikced();
		
		InitService();
		InitTab();
	}

	void CTiltModelStyleWidget::InitTab()
	{
		
		SetCenture(m_pTiltModelNode->GetPosition());
		SetAngle(m_pTiltModelNode->GetRotate().z());
		SetScale(m_pTiltModelNode->GetScale().z());

		switch (m_pTiltModelNode->GetOption()->tileType())
		{
		case FeExtNode::CExTiltModelNodeOption::E_TILT_TYPE_FILEPATH:
			{
				ui.radioButton_filePath->setChecked(true);
				SlotLocalRadioClikced();
				SetRS(m_pTiltModelNode->GetTiltPath());
			}
			break;
		case FeExtNode::CExTiltModelNodeOption::E_TILT_TYPE_DTATPATH:
			{
				ui.radioButton_dataPath->setChecked(true);
				SlotLocalRadioClikced();
				SetRS(m_pTiltModelNode->GetTiltPath());
			}
			break;
		case FeExtNode::CExTiltModelNodeOption::E_TILT_TYPE_SERVICE:
			{
				ui.lineEdit_service_path->setText(m_pTiltModelNode->GetTiltPath().c_str());
				ui.radioButton_service->setChecked(true);
				SlotServiceRadioClicked();
				if (m_bInit)
				{
						ui.comboBox_service->setCurrentIndex(ui.comboBox_service->count()-1);
				}
				ui.lineEdit_service_path->setStyleSheet("color : white");
				m_bServicePathChanged = true;
			}
			break;
		}
		m_bInit = false;
	}

	CTiltModelStyleWidget::~CTiltModelStyleWidget()
	{
	}

	void CTiltModelStyleWidget::SetRS( const std::string& strRS )
	{
		ui.lineEdit_path->setText(strRS.c_str());
	}

	std::string CTiltModelStyleWidget::GetRS() const
	{
		return ui.lineEdit_path->text().toStdString();
	}

	void CTiltModelStyleWidget::SetAngle( double dAngle )
	{
		ui.doubleSpinBox_Angle->setValue(dAngle);
	}

	double CTiltModelStyleWidget::GetAngle() const
	{
		return ui.doubleSpinBox_Angle->value();
	}

	void CTiltModelStyleWidget::SetScale( double dScale )
	{
		ui.doubleSpinBox_Scale->setValue(dScale);
	}

	double CTiltModelStyleWidget::GetScale() const
	{
		return ui.doubleSpinBox_Scale->value();
	}

	void CTiltModelStyleWidget::SetCenture( const osg::Vec3& center )
	{
		ui.doubleSpinBox_Lon->setValue(center.x());
		ui.doubleSpinBox_Lat->setValue(center.y());
		ui.doubleSpinBox_Hight->setValue(center.z());
		if(m_bCreate)
		{
			m_pTiltModelNode->SetPosition(center);
		}

		if (m_bCreate)
		{
			double dRadius = m_pTiltModelNode->getBound().radius() * 2;
			double height = dRadius / tan(osg::DegreesToRadians(30.0));
			emit SignalViewPointChanged(FeUtil::CFreeViewPoint(center.x(), center.y(), center.z() + height, -90.0, -90.0, 10000, 1));
		}
	}

	bool CTiltModelStyleWidget::Reject()
	{
		if(m_pTiltModelNode)
		{
			m_pTiltModelNode->SetPosition(m_markOpt.tiltConfig().position());
			m_pTiltModelNode->SetScale(m_markOpt.tiltConfig().scale());
			m_pTiltModelNode->SetRotate(m_markOpt.tiltConfig().rotate());
			m_pTiltModelNode->SetTiltPath(m_markOpt.tiltPath());
			m_pTiltModelNode->SetTiltType(m_markOpt.tileType());
		}
		return true;
	}

	osg::Vec3 CTiltModelStyleWidget::GetCenture() const
	{
		return osg::Vec3d(ui.doubleSpinBox_Lon->value(), ui.doubleSpinBox_Lat->value(), ui.doubleSpinBox_Hight->value());
	}

	void CTiltModelStyleWidget::SlotLonChanged( double dLong)
	{
		osg::Vec3d vecLLH = m_pTiltModelNode->GetPosition();
		vecLLH.x() = dLong;
		m_pTiltModelNode->SetPosition(vecLLH);

		if (m_bCreate)
		{
			CFreeViewPoint view = m_pTiltModelNode->GetViewPoint();
			view = FeUtil::CFreeViewPoint(dLong, view.GetLat(), view.GetHei(), view.GetHeading(), view.GetPitch(), view.GetRange(), view.GetTime());
			m_pTiltModelNode->SetViewPoint(view);
			emit SignalViewPointChanged(view);
		}
	}

	void CTiltModelStyleWidget::SlotLatChanged( double dLat)
	{
		osg::Vec3d vecLLH = m_pTiltModelNode->GetPosition();
		vecLLH.y() = dLat;
		m_pTiltModelNode->SetPosition(vecLLH);

		if (m_bCreate)
		{
			CFreeViewPoint view = m_pTiltModelNode->GetViewPoint();
			view = FeUtil::CFreeViewPoint(view.GetLon(), dLat, view.GetHei(), view.GetHeading(), view.GetPitch(), view.GetRange(), view.GetTime());
			m_pTiltModelNode->SetViewPoint(view);
			emit SignalViewPointChanged(view);
		}
	}

	void CTiltModelStyleWidget::SlotHeightChanged( double dHeight)
	{
		osg::Vec3d vecLLH = m_pTiltModelNode->GetPosition();
		CFreeViewPoint view = m_pTiltModelNode->GetViewPoint();
		double dViewHeight = view.GetHei() - vecLLH.z();

		vecLLH.z() = dHeight;
		m_pTiltModelNode->SetPosition(vecLLH);

		if (m_bCreate)
		{
			view = FeUtil::CFreeViewPoint(view.GetLon(), view.GetLat(), dViewHeight + dHeight, view.GetHeading(), view.GetPitch(), view.GetRange(), view.GetTime());
			m_pTiltModelNode->SetViewPoint(view);
			emit SignalViewPointChanged(view);
		}
	}

	void CTiltModelStyleWidget::SlotScaleChanged( double dScale )
	{
		m_pTiltModelNode->SetScale(osg::Vec3d(dScale, dScale, dScale));
	}

	void CTiltModelStyleWidget::SlotAngleChanged( double dAngle)
	{
		m_pTiltModelNode->SetRotate(osg::Vec3d(m_pTiltModelNode->GetRotate().x(), m_pTiltModelNode->GetRotate().y(), dAngle));
	}

	void CTiltModelStyleWidget::BrowseConfigPath()
	{
		QString strCaption = tr("Open File");
		QString strDir = tr(FeFileReg->GetDataPath().c_str());
		QString filePath;
#ifdef WIN32
		QFileDialog* pFileDialog = new QFileDialog(0, strCaption, strDir, tr("model Files(*.*)"));
		pFileDialog->setAttribute(Qt::WA_DeleteOnClose, true);
		filePath = pFileDialog->getOpenFileName();
		if(!filePath.isNull())
		{
			ui.lineEdit_path->setText(filePath);
		}
#else
		QFileDialog fileDialog(0, strCaption, strDir, tr("model Files(*.*)"));
		if (fileDialog.exec())
		{
			filePath = fileDialog.selectedFiles().first();
            ui.lineEdit_path->setText(filePath);
		}
#endif
		FormatPathString(filePath);
		m_pTiltModelNode->SetTiltPath(filePath.toStdString());
		m_pTiltModelNode->SetTiltType(FeExtNode::CExTiltModelNodeOption::E_TILT_TYPE_FILEPATH);
	}

	void CTiltModelStyleWidget::BrowseDataPath()
	{
		// 读取数据路径
		QString strCaption = tr("Data Path");
		QString strDir = tr(FeFileReg->GetDataPath().c_str());
		QString filePath;

#ifdef WIN32
		QFileDialog* pFileDialog = new QFileDialog(0, strCaption, strDir);
		filePath = pFileDialog->getExistingDirectory(this, strCaption, strDir);
		if(!filePath.isNull())
		{
			ui.lineEdit_path->setText(filePath);
		}
#else
		QFileDialog fileDialog(0, strCaption, strDir);
		fileDialog.setFileMode( QFileDialog::DirectoryOnly);
		if (fileDialog.exec())
		{
            filePath = fileDialog.selectedFiles().first();
            ui.lineEdit_path->setText(filePath);
		}
#endif
		FormatPathString(filePath);
		m_pTiltModelNode->GenerateData(filePath.toStdString());
		m_pTiltModelNode->SetTiltType(FeExtNode::CExTiltModelNodeOption::E_TILT_TYPE_DTATPATH);
	}

	void CTiltModelStyleWidget::SlotBorwseBtnClicked( bool )
	{
		if (ui.radioButton_filePath->isChecked())
		{
			BrowseConfigPath();
		}
		else if (ui.radioButton_dataPath->isChecked())
		{
			BrowseDataPath();
		}

		InitTab();
	}

	void CTiltModelStyleWidget::ShowServericUi( bool bShow)
	{
		ui.btn_browse->setVisible(!bShow);
		ui.lineEdit_path->setVisible(!bShow);
		ui.pushButton_link->setVisible(bShow);
		ui.comboBox_service->setVisible(bShow);
		ui.lineEdit_service_path->setVisible(bShow);
		ui.label_more->setVisible(bShow);
		ui.label_website->setVisible(bShow);
	}

	void CTiltModelStyleWidget::SlotLocalRadioClikced()
	{
		ShowServericUi(false);
	}

	void CTiltModelStyleWidget::SlotServiceRadioClicked()
	{
		ShowServericUi(true);
	}

	void CTiltModelStyleWidget::SlotServicePath( int index)
	{
		QString strUrl = ui.comboBox_service->itemData(index).toString();
		ui.lineEdit_service_path->setText(strUrl);
		if (ui.comboBox_service->count()-1 == index)
		{
			ui.lineEdit_service_path->setReadOnly(false);
			m_bServicePathChanged = false;
			ui.lineEdit_service_path->clear();
			ui.lineEdit_service_path->setText(tr("please input the model path"));
			ui.lineEdit_service_path->setStyleSheet("color : gray");
		}
		else
		{
			ui.lineEdit_service_path->setReadOnly(true);
			ui.lineEdit_service_path->setStyleSheet("color : white");
			m_bServicePathChanged = true;
		}
	}

	void CTiltModelStyleWidget::InitService()
	{
		ui.comboBox_service->blockSignals(true);
		ui.comboBox_service->addItem("大雁塔", "http://39.104.75.125:8090/FreeServer/model/osgb/dayanta?subName=dayanta.fgt");
		ui.comboBox_service->addItem(tr("custom"));
		ui.comboBox_service->blockSignals(false);
		ui.label_more->setText(tr("more service model please visite :"));
		ui.label_more->setStyleSheet("color: gray;");
		ui.comboBox_service->setCurrentIndex(ui.comboBox_service->count()-1);
	}

	void CTiltModelStyleWidget::SlotServicePathLinked()
	{
		QString strUrl = ui.lineEdit_service_path->text();
		m_pTiltModelNode->SetServiceTiltPath(strUrl.toStdString());
		m_pTiltModelNode->SetTiltType(FeExtNode::CExTiltModelNodeOption::E_TILT_TYPE_SERVICE);
		InitTab();
	}

	void CTiltModelStyleWidget::SlotUrlEditFinished()
	{
		if ("" == ui.lineEdit_service_path->text())
		{
			ui.lineEdit_service_path->setText(tr("please input the model path"));
			ui.lineEdit_service_path->setStyleSheet("color : gray");
			m_bServicePathChanged = false;
		}
	}

	void CTiltModelStyleWidget::SlotUrlClicked()
	{
		if (!m_bServicePathChanged)
		{
			ui.lineEdit_service_path->clear();
			ui.lineEdit_service_path->setStyleSheet("color : white");
			m_bServicePathChanged = true;
		}
	}

	void CTiltModelStyleWidget::SlotServicePathChanged( const QString& )
	{
		if (!m_bServicePathChanged)
		{
			m_bServicePathChanged = true;
			ui.lineEdit_service_path->setStyleSheet("color : white");
		}
	}

}

