#include "BootScreenFrame.h"

namespace FreeViewer
{
	CBootScreenFrame::CBootScreenFrame(QWidget *parent)
		: QSplashScreen(parent)
		, m_pRoundBar(NULL)
		, m_pBallPath(NULL)
		, m_nStartVal(0)
		, m_nEndVal(0)
	{
		ui.setupUi(this);

		InitBootSceen();
	}

	void CBootScreenFrame::InitBootSceen()
	{
		ui.main_widget->setStyleSheet("QWidget#main_widget { image: url(:/images/boot_screen/boot_screen.png);}");
		ui.flash_widget->setStyleSheet("image: url(:/images/boot_screen/earth_screen.png);");

		m_effect = new QGraphicsOpacityEffect(ui.flash_widget);


		setWindowOpacity(1); //设置透明度
		setWindowFlags(Qt::FramelessWindowHint); //设置无边框
		setAttribute(Qt::WA_TranslucentBackground, true); //透明背景
		setDisabled(true); //禁用用户的输入事件响应

		//实例化圆形进度条
		m_pRoundBar = new CBootScreenRoundBar;

		ui.gridLayout_roundBar->addWidget(m_pRoundBar);
		LoadRonudBar(); //加载圆形进度条

		//实例化小球沿路径移动
		m_pBallPath = new CBootScreenBallPath;
		ui.gridLayout_ballPath->addWidget(m_pBallPath);

		m_timer = new QTimer;
		m_nTimerCount = 1;
		connect(m_timer, SIGNAL(timeout()), this, SLOT(SetFlashImage()));
		m_timer->start(2500);
	}

	void CBootScreenFrame::SetLoadText(QString str1, QString str2, QString str3)
	{
		ui.label_first->clear();
		ui.label_second->clear();
		ui.label_third->clear();
		ui.label_first->setText(str1);
		ui.label_second->setText(str2);
		ui.label_third->setText(str3);
		repaint();
	}

	void CBootScreenFrame::SetFlashImage()
	{
		m_nTimerCount++;

		if (m_nTimerCount > 3)
		{
			m_nTimerCount = 1;
		}

		switch (m_nTimerCount)
		{
		case 1:
			SetWidgetOpacity(false);
			ui.flash_widget->setStyleSheet("image: url(:/images/boot_screen/earth_screen.png);");
			SetWidgetOpacity(true);
			break;
		case 2:
			SetWidgetOpacity(false);
			ui.flash_widget->setStyleSheet("image: url(:/images/boot_screen/plane_screen.png);");
			SetWidgetOpacity(true);
			break;
		case 3:
			SetWidgetOpacity(false);
			ui.flash_widget->setStyleSheet("image: url(:/images/boot_screen/satellite_screen.png);");
			SetWidgetOpacity(true);
			break;
		default:
			break;
		}
	}

	void CBootScreenFrame::SetWidgetOpacity(bool bIN)
	{
		if (bIN)
		{
			for (int i = 1; i<=10; i++)
			{
				double di = (i * 1.0) / 10;
				m_effect->setOpacity(di);
				ui.flash_widget->setGraphicsEffect( m_effect );

				//sleep(50);
				repaint();
			}
		}
		else
		{
			for (int i=10; i>=1; i--)
			{
				double di = (i * 1.0) /10;
				m_effect->setOpacity(di);
				ui.flash_widget->setGraphicsEffect(m_effect);

				//sleep(50);
				repaint();
			}
		}
	}

	void CBootScreenFrame::SetRoundProgress(int start_value, int end_value)
	{
		m_nStartVal = start_value;
		m_nEndVal = end_value;

		for(;m_nStartVal < m_nEndVal; m_nStartVal++)
		{
			m_pRoundBar->setValue(m_nStartVal);
			sleep(1);
		}
	}

	void CBootScreenFrame::SetRoundProgress(int start_value, int end_value, bool state)
	{
		if (state) //线程结束
		{
			for(;m_nStartVal < m_nEndVal; m_nStartVal++)
			{
				m_pRoundBar->setValue(m_nStartVal);
				sleep(1);
			}
		}
		else //线程没有结束
		{
			m_nStartVal = start_value;
			m_nEndVal = end_value;

			for(;m_nStartVal < m_nEndVal; m_nStartVal++)
			{
				m_pRoundBar->setValue(m_nStartVal);
				sleep(100);

				if (m_nStartVal == m_nEndVal-1)
				{
					m_nStartVal--;
				}
			}
		}
	}

	void CBootScreenFrame::LoadRonudBar()
	{
		QPalette palette;
		palette.setBrush(QPalette::Highlight, QBrush(QColor(0, 160, 233))); //填充进度条区域的背景色
		palette.setColor(QPalette::Text, Qt::white); //在中心显示的文本色
		m_pRoundBar->setPalette(palette);
		m_pRoundBar->setBarStyle(CBootScreenRoundBar::StyleLine); //设置显示风格
		m_pRoundBar->setDecimals(0); //显示小数点位数
		m_pRoundBar->setOutlinePenWidth(20); //设置圆环轮廓线的画笔宽度
		m_pRoundBar->setDataPenWidth(10); //设置圆环数据圆笔的宽度
		m_pRoundBar->setRange(0, 100); //设置进度条范围
		m_pRoundBar->setFormat("%v");
		m_pRoundBar->setValue(0); //设置进度条值
	}

	void CBootScreenFrame::sleep(unsigned int msec)
	{ 
		QTime dieTime = QTime::currentTime().addMSecs(msec); 
		while( QTime::currentTime() < dieTime )
		{
			QCoreApplication::processEvents(QEventLoop::AllEvents, 100); 
		}
	}

	CBootScreenFrame::~CBootScreenFrame()
	{
		if (m_pRoundBar)
		{
			delete m_pRoundBar;
			m_pRoundBar = NULL;
		}
	}
}

