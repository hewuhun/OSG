#include <tool/LanguageWidget.h>

#include <FeUtils/PathRegistry.h>

#include <QMessageBox>
#include <QProcess>

namespace FreeViewer
{
	using namespace FeShell;

	CLanguageWidget::CLanguageWidget(QWidget* parent)
		: CFreeDialog(parent)
	{
		//添加ui界面到属性窗体客户区
		QWidget *widget = new QWidget(this);
		ui.setupUi(widget);
		AddWidgetToDialogLayout(widget);
		widget->setFixedSize(widget->geometry().size());

		InitWidget();
	}

	CLanguageWidget::~CLanguageWidget()
	{

	}

	void CLanguageWidget::InitWidget()
	{
		BuildContext();

		connect(ui.comboBox_Language, SIGNAL(currentIndexChanged(QString)), this, SLOT(SlotLanguageChanged(QString)));
		connect(ui.btn_OK, SIGNAL(released()), this, SLOT(SlotOK()));
		connect(ui.btn_Cancel, SIGNAL(released()), this, SLOT(SlotCancel()));
	}

	void CLanguageWidget::BuildContext()
	{
		SetTitleText(tr("LanguageSet"));
		//清空列表
		ui.comboBox_Language->clear();

		//加载xml文件数据
		CLanConfigReader load;
		m_lanConfig = load.Execute(FeFileReg->GetFullPath("apps/language/lanConfig.xml"));

		//加载语言列表
		CLanConfig::LanMap lanMap = m_lanConfig.GetLanguageList();
		m_strDefLan = m_lanConfig.GetDefLanguage().c_str();
		if ("Chinese" == m_strDefLan)
		{
			ui.comboBox_Language->insertItem(0, tr("Chinese"));
		}
		else if ("English" == m_strDefLan)
		{
			ui.comboBox_Language->insertItem(0, tr("English"));
		}

		//遍历数据并加载到下拉框
		CLanConfig::LanMap::iterator itLan = lanMap.begin();
		while(itLan != lanMap.end())
		{
			if (0 != m_lanConfig.GetDefLanguage().compare(itLan->first.c_str()))
			{
				QString strLanguage = itLan->first.c_str();
				if ("Chinese" == strLanguage)
				{
					ui.comboBox_Language->addItem(tr("Chinese"));
				}
				else if ("English" == strLanguage)
				{
					ui.comboBox_Language->addItem(tr("English"));
				}
			}
			itLan++;
		}
	}

	void CLanguageWidget::SlotLanguageChanged( QString strKey )
	{
		if (QString("英文") == strKey)
		{
			strKey = "English";
		}
		else if (QString("中文") == strKey)
		{
			strKey = "Chinese";
		}
		m_lanConfig.SetDefLanguage(strKey.toStdString());
	}

	void CLanguageWidget::SlotOK()
	{
		ApplyLanguageChange(m_lanConfig);

		QString strCurrent = ui.comboBox_Language->currentText();

		if (strCurrent != m_strDefLan)
		{
			QMessageBox message(QMessageBox::Warning, tr("Exit"), tr("Really to Restart system?"), QMessageBox::Yes | QMessageBox::No, this);
			message.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
			message.setButtonText(QMessageBox::Yes, tr("Restart Now"));
			message.setButtonText(QMessageBox::No, tr("Restart Later"));
			if (message.exec()==QMessageBox::Yes)  
			{  
				QString strProcessPath = QApplication::applicationFilePath();
				QApplication::exit();
				//创建另一个进程
				QProcess::startDetached(strProcessPath);
			}  	
			else
			{
				close();
			}
		}
		else
		{
			close();
		}
	} 

	void CLanguageWidget::SlotCancel()
	{
		ApplyLanguageChange(m_preLanConfig);
		close();
	}

	void CLanguageWidget::show()
	{
		m_preLanConfig = m_lanConfig;
		
		ShowDialogNormal();
	}

	void CLanguageWidget::ApplyLanguageChange(const FeShell::CLanConfig& config)
	{
		CLanConfigWriter write;
		write.Execute(config);
		
	}

}

