#include <mark/ParticleStyleWidget.h>
#include <FeUtils/PathRegistry.h>
#include <FeUtils/CoordConverter.h>
#include <FeUtils/EventListener.h>
#include <QFileDialog>

namespace FreeViewer
{
	CParticleStyleWidget::CParticleStyleWidget(FeExtNode::CExParticleNode* opMark, QWidget *parent)
		: CBaseStyleWidget(parent)
		, m_pParticleNode(opMark)
	{
		ui.setupUi(this);

		setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

		InitWidget();
	}

	void CParticleStyleWidget::InitWidget()
	{
		setWindowTitle(tr("Particle"));

		//隐藏angle组
		//ui.groupBox_angle->hide();

		m_pParticleNodeOpt = *(m_pParticleNode->GetOption());

		ui.dSpinBox_longitude->setValue(m_pParticleNode->GetParticlePosition().x());
		ui.dSpinBox_latitude->setValue(m_pParticleNode->GetParticlePosition().y());
		ui.dSpinBox_height->setValue(m_pParticleNode->GetParticlePosition().z());

		ui.dSpinBox_angleX->setValue(m_pParticleNode->GetParticleAngle().x());
		ui.dSpinBox_angleY->setValue(m_pParticleNode->GetParticleAngle().y());
		ui.dSpinBox_angleZ->setValue(m_pParticleNode->GetParticleAngle().z());

		ui.dSpinBox_Scale->setValue(m_pParticleNode->GetParticleScale());

		ui.lineEdit_path->setText(m_pParticleNode->GetParticlePath().c_str());

		connect(ui.dSpinBox_longitude, SIGNAL(valueChanged(double)), this, SLOT(SlotLongitudeChanged(double)));
		connect(ui.dSpinBox_latitude, SIGNAL(valueChanged(double)), this, SLOT(SlotLatitudeChanged(double)));
		connect(ui.dSpinBox_height, SIGNAL(valueChanged(double)), this, SLOT(SlotHeightChanged(double)));
		connect(ui.dSpinBox_angleX, SIGNAL(valueChanged(double)), this, SLOT(SlotAngleXChanged(double)));
		connect(ui.dSpinBox_angleY, SIGNAL(valueChanged(double)), this, SLOT(SlotAngleYChanged(double)));
		connect(ui.dSpinBox_angleZ, SIGNAL(valueChanged(double)), this, SLOT(SlotAngleZChanged(double)));

		connect(ui.dSpinBox_Scale, SIGNAL(valueChanged(double)), this, SLOT(SlotScaleChanged(double)));

		connect(ui.btn_path, SIGNAL(clicked()), this, SLOT(SlotPathBtnClicked()));
	}

	bool CParticleStyleWidget::Reject()
	{
		if(m_pParticleNode.valid())
		{
			m_pParticleNode->SetParticlePath(m_pParticleNodeOpt.GetPath());
			m_pParticleNode->SetParticleAngle(m_pParticleNodeOpt.GetAngle());
			m_pParticleNode->SetParticlePosition(m_pParticleNodeOpt.GetPosition());
		}
		return true;
	}

	void CParticleStyleWidget::SlotLongitudeChanged(double value)
	{
		SetParticleCenter();
	}

	void CParticleStyleWidget::SlotLatitudeChanged(double value)
	{
		SetParticleCenter();
	}

	void CParticleStyleWidget::SlotHeightChanged(double value)
	{
		SetParticleCenter();
	}

	void CParticleStyleWidget::SlotAngleXChanged(double x)
	{
		SetParticleAngle();
	}

	void CParticleStyleWidget::SlotAngleYChanged(double y)
	{
		SetParticleAngle();
	}

	void CParticleStyleWidget::SlotAngleZChanged(double z)
	{
		SetParticleAngle();
	}

	void CParticleStyleWidget::SlotPathBtnClicked()
	{	
		QString filePath;
		QString dirPath = FeFileReg->GetDataPath().c_str();
		dirPath += "/particle/particle";
#ifdef WIN32
		QFileDialog* pFileDialog = new QFileDialog();
		pFileDialog->setAttribute(Qt::WA_DeleteOnClose, true);
		filePath = pFileDialog->getOpenFileName(0, tr("open File"), dirPath, tr("Particle Files(*.xml)"));
		if(!filePath.isNull())
		{
			ui.lineEdit_path->setText(filePath);
		}
#else
		QFileDialog fileDialog(0, tr("open File"), dirPath, tr("particle Files(*.xml)"));
		if (fileDialog.exec())
		{
			filePath = fileDialog.selectedFiles().first();
			ui.lineEdit_path->setText(filePath);
		}
#endif
		m_pParticleNode->SetParticlePath(filePath.toStdString());
		SetParticleCenter();
		SetParticleAngle();
	}

	void CParticleStyleWidget::SetParticleCenter()
	{
		//获取粒子位置
		double dLongitude = ui.dSpinBox_longitude->value();
		double dLatitude = ui.dSpinBox_latitude->value();
		double dHeight = ui.dSpinBox_height->value();

		osg::Vec3d vecLLH(dLongitude, dLatitude, dHeight);

		//设置粒子位置
		m_pParticleNode->SetParticlePosition(vecLLH);
	}

	void CParticleStyleWidget::SetParticleAngle()
	{
		//获取粒子角度
		double dAngleX = ui.dSpinBox_angleX->value();
		double dAngleY = ui.dSpinBox_angleY->value();
		double dAngleZ = ui.dSpinBox_angleZ->value();

		osg::Vec3 vecAngle(dAngleX, dAngleY, dAngleZ);

		//设置粒子角度
		m_pParticleNode->SetParticleAngle(vecAngle);
	}

	CParticleStyleWidget::~CParticleStyleWidget()
	{

	}

	void CParticleStyleWidget::SlotScaleChanged( double dScale )
	{
		if (m_pParticleNode.valid())
		{
			m_pParticleNode->SetParticleScale(dScale);
		}
	}

}

