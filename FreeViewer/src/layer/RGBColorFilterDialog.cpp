#include <layer/RGBColorFilterDialog.h>

namespace FreeViewer
{
	using namespace osgEarth;

	CRGBColorFilterDialog::CRGBColorFilterDialog(FeLayers::CLayerSys* pLayerSys, QWidget *parent)
		:CFreeDialog(parent)
		,m_opLayerSys(pLayerSys)
		,m_rpRGBColorFilter(NULL)
		,m_rpSTFColorFilter(NULL)
	{
		//添加ui界面到属性窗体客户区
		QWidget *widget = new QWidget(this);
		ui.setupUi(widget);
		AddWidgetToDialogLayout(widget);
		widget->setFixedSize(widget->geometry().size());
		
		SetTitleText(tr("Color Filter"));

		InitWidget();
	}

	CRGBColorFilterDialog::~CRGBColorFilterDialog()
	{

	}

	void CRGBColorFilterDialog::InitWidget()
	{
		connect(ui.checkBox_NostalgiaVisible, SIGNAL(clicked(bool)), this, SLOT(SlotNostalgiaVisible(bool)));
		connect(ui.checkBox_RGBVisible, SIGNAL(clicked(bool)), this, SLOT(SlotRGBColorFilterVisible(bool)));
		connect(ui.hSlider_Red, SIGNAL(valueChanged(int)), this, SLOT(SlotRGBColorChanged(int)));
		connect(ui.hSlider_Green, SIGNAL(valueChanged(int)), this, SLOT(SlotRGBColorChanged(int)));
		connect(ui.hSlider_Blue, SIGNAL(valueChanged(int)), this, SLOT(SlotRGBColorChanged(int)));

		ui.label_Red->setText(QString::number(ui.hSlider_Red->value()));
		ui.label_Green->setText(QString::number(ui.hSlider_Green->value()));
		ui.label_Blue->setText(QString::number(ui.hSlider_Blue->value()));

		UpdateCtrlState();
	}

	void CRGBColorFilterDialog::UpdateCtrlState()
	{
		if(ui.checkBox_RGBVisible->isChecked())
		{
			ui.label_Red->setEnabled(true);
			ui.label_Green->setEnabled(true);
			ui.label_Blue->setEnabled(true);
			ui.hSlider_Red->setEnabled(true);
			ui.hSlider_Green->setEnabled(true);
			ui.hSlider_Blue->setEnabled(true);
		}
		else
		{
			ui.label_Red->setEnabled(false);
			ui.label_Green->setEnabled(false);
			ui.label_Blue->setEnabled(false);
			ui.hSlider_Red->setEnabled(false);
			ui.hSlider_Green->setEnabled(false);
			ui.hSlider_Blue->setEnabled(false);
		}
	}

	void CRGBColorFilterDialog::SlotNostalgiaVisible( bool bState )
	{
		if (false == m_opLayerSys.valid())
		{
			return;
		}

		if (bState)
		{
			if (false == m_rpSTFColorFilter.valid())
			{
				m_rpSTFColorFilter = new FeLayers::CSTFColorFilter();
			}
			ui.checkBox_RGBVisible->setChecked(false);
			SlotRGBColorFilterVisible(false);
			m_opLayerSys->AddImageColorFilter(m_rpSTFColorFilter);
		}
		else
		{
			if (m_rpSTFColorFilter.valid())
			{
				m_opLayerSys->RemoveImageColorFilter(m_rpSTFColorFilter);
			}
		}
	}

	void CRGBColorFilterDialog::SlotRGBColorFilterVisible( bool bState )
	{
		if (false == m_opLayerSys.valid())
		{
			return;
		}

		if (bState)
		{
			if (false == m_rpRGBColorFilter.valid())
			{
				m_rpRGBColorFilter = new osgEarth::Util::RGBColorFilter;
			}
			ui.checkBox_NostalgiaVisible->setChecked(false);
			SlotNostalgiaVisible(false);
			m_opLayerSys->AddImageColorFilter(m_rpRGBColorFilter);
		}
		else
		{
			if (m_rpRGBColorFilter.valid())
			{
				m_opLayerSys->RemoveImageColorFilter(m_rpRGBColorFilter);
			}
		}

		UpdateCtrlState();
	}

	void CRGBColorFilterDialog::SlotRGBColorChanged(int)
	{
		if (m_rpRGBColorFilter.valid())
		{
			m_rpRGBColorFilter->setRGBOffset(osg::Vec3f(ui.hSlider_Red->value()/255.0, ui.hSlider_Green->value()/255.0, ui.hSlider_Blue->value()/255.0));
		}

		ui.label_Red->setText(QString::number(ui.hSlider_Red->value()));
		ui.label_Green->setText(QString::number(ui.hSlider_Green->value()));
		ui.label_Blue->setText(QString::number(ui.hSlider_Blue->value()));
	}

}
