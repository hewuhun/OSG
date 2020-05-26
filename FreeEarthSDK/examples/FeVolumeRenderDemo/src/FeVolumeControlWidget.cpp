#include <FeVolumeControlWidget.h>

#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QVBoxLayout>

CVolumeRenderControlPan::~CVolumeRenderControlPan()
{

}

CVolumeRenderControlPan::CVolumeRenderControlPan( FeVolumeRender::CVolumeRenderNode* pNode, QWidget* parent )
	:QWidget(parent)
	,m_opVolumeRenderNode(pNode)
{
	InitWidget();
}

void CVolumeRenderControlPan::InitWidget()
{
	QVBoxLayout* pLayout = new QVBoxLayout(this);

	//{
	//	QHBoxLayout* pStepSizeLayout = new QHBoxLayout();
	//	pLayout->addLayout(pStepSizeLayout);
	//	pStepSizeLayout->addWidget(new QLabel("采样步长"));
	//	QDoubleSpinBox* pDoubleSpin = new QDoubleSpinBox(this);
	//	pDoubleSpin->setMaximum(0.1);
	//	pDoubleSpin->setMinimum(0.001);
	//	pDoubleSpin->setSingleStep(0.001);
	//	pDoubleSpin->setValue(0.001);
	//	pDoubleSpin->setDecimals(3);
	//	pDoubleSpin->setContextMenuPolicy(Qt::NoContextMenu);
	//	pStepSizeLayout->addWidget(pDoubleSpin);

	//	bool bState = connect(pDoubleSpin, SIGNAL(valueChanged(double)), this, SLOT(SlotStepChanged(double)));
	//}

	{
		QHBoxLayout* pStepSizeLayout = new QHBoxLayout();
		pLayout->addLayout(pStepSizeLayout);
		pStepSizeLayout->addWidget(new QLabel(QString::fromLocal8Bit("采样点数")));
		QSpinBox* pIntSpinBox = new QSpinBox(this);
		pIntSpinBox->setMaximum(3000);
		pIntSpinBox->setMinimum(10);
		pIntSpinBox->setSingleStep(10);
		pIntSpinBox->setValue(1600);
		pIntSpinBox->setContextMenuPolicy(Qt::NoContextMenu);
		pStepSizeLayout->addWidget(pIntSpinBox);
	
		bool bState = connect(pIntSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SlotSampleNumChanged(int)));
	}
}

void CVolumeRenderControlPan::SlotStepChanged( double dStep )
{
	if (m_opVolumeRenderNode.valid())
	{
		m_opVolumeRenderNode->SetStepSize(dStep);
	}
}

void CVolumeRenderControlPan::SlotSampleNumChanged( int nSampleNum )
{
	double dStepSize = 1.6/double(nSampleNum);

	if (m_opVolumeRenderNode.valid())
	{
		m_opVolumeRenderNode->SetSampleNum(nSampleNum);
		m_opVolumeRenderNode->SetStepSize(dStepSize);
	}
}

