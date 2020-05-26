#include <view/ViewPluginInterface.h>

#include <FeUtils/PathRegistry.h>
#include <FeShell/SmartToolSys.h>
#include <FeKits/airview/AirView.h>
#include <FeKits/mapScale/MapScale.h>
#include <FeKits/navigator/Navigator.h>
#include <FeKits/navigator/EarthManipulatorCtrl.h>
#include <FeKits/IR/SulIR.h>
#include <FeKits/graticule/FeGraticule.h>
#include <FeKits/manipulator/ManipulatorUtil.h>

#include <view/FreeStatusCallback.h>

#include <mainWindow/FreeMainWindow.h>
#include <mainWindow/FreeMenuWidget.h>
#include <mainWindow/FreeToolBar.h>
#include <mainWindow/FreeUtil.h>

#include <QTextCodec>

namespace FreeViewer
{
	CKitViewUnit::CKitViewUnit( FeKit::CKitsCtrl* pKitsCtrl, QObject* parent )
		:QObject(parent)
		,m_pKitsCtrl(pKitsCtrl)
	{

	}

	CKitViewUnit::~CKitViewUnit()
	{

	}

	void CKitViewUnit::SlotDoAction( bool bState )
	{
		if(m_pKitsCtrl)
		{
			if (bState)
			{
				m_pKitsCtrl->Show();
			}
			else
			{
				m_pKitsCtrl->Hide();
			}
		}
	}
}

namespace FreeViewer
{
	CViewPluginInterface::CViewPluginInterface( CFreeMainWindow* pMainWindow)
		:CUIObserver(pMainWindow)
		,m_pSceneViewInfo(NULL)
	{
		m_strMenuTitle = QString(tr("Tool"));

		InitWidget();		
	}

	CViewPluginInterface::~CViewPluginInterface()
	{

	}

	void CViewPluginInterface::InitWidget()
	{
		BuildContext();
	}

	void CViewPluginInterface::BuildContext()
	{
		if(m_opSystemService.valid() && m_pMainWindow)
		{
			osg::observer_ptr<FeShell::CSmartToolSys> opSystem =  m_opSystemService->GetSmartToolSys();
			if(opSystem.valid())
			{
 				//状态栏显示信息
 				{
 					m_pSceneViewInfo = new FeKit::CSceneViewInfo(m_opSystemService->GetRenderContext());
 
 					osg::ref_ptr<CFreeStatusInfoCallback> pStatusInfoCallback = new CFreeStatusInfoCallback(m_pMainWindow->GetStatusBar());
 					m_pSceneViewInfo->AddCallback(pStatusInfoCallback);
 				}
 
 				///鸟瞰图
 				{
 					std::string strTitle = QString(tr("AirView")).toStdString();
 					osg::ref_ptr<FeKit::CAirView> pKit = new FeKit::CAirView(
 						m_opSystemService->GetRenderContext(),
 						10, 
 						10,
 						280, 
 						140,
 						FeKit::CPlaceStrategy::RIGHT_BOTTOM,
 						FeFileReg->GetFullPath("texture/airView/land_shallow_topo_2048.jpg"));
 					pKit->SetKey(strTitle);
 					pKit->SetTitle(strTitle);
 
 					if(opSystem->AddSmartTool(pKit.get()))
 					{
 						BuildKitAction(
 							pKit,
 							":/images/icon/bird_eye.png",
 							":/images/icon/bird_eye.png",
 							":/images/icon/bird_eye_press.png",
 							":/images/icon/bird_eye.png");
 					}
 				}
 
 				///显示导航器，操作地球
 				{
 					std::string strNV = FeFileReg->GetFullPath("texture/navigator");
 					std::string strTitle = QString(tr("Navigator")).toStdString();
 					osg::ref_ptr<FeKit::CNavigator> pKit = new FeKit::CNavigator(
 						m_opSystemService->GetRenderContext(),
 						10,
 						10,
 						90, 
 						360, 
 						FeKit::CPlaceStrategy::RIGHT_TOP,
 						new FeKit::CEarthManipulatorCtrl(m_opSystemService->GetManipulatorManager()->GetOrCreateFEManipulator()),
 						strNV);
 					pKit->SetKey(strTitle);
 					pKit->SetTitle(strTitle);
 
 					if(opSystem->AddSmartTool(pKit.get()))
 					{
 						BuildKitAction(
 							pKit,
 							":/images/icon/navigator.png",
 							":/images/icon/navigator.png",
 							":/images/icon/navigator_press.png", 
 							":/images/icon/navigator.png");
 					}
 				}
 
 				///创建比例尺
 				{
 					std::string strTitle = QString(tr("MapScale")).toStdString();
 					osg::ref_ptr<FeKit::CMapScale> pKit = new FeKit::CMapScale(
 						m_opSystemService->GetRenderContext(),
 						10,
 						10, 
 						300, 
 						50,
 						FeKit::CPlaceStrategy::LEFT_BOTTOM);
 					pKit->SetKey("MapScale");
 					pKit->SetTitle(strTitle);
 					pKit->Show();
 
 					if(opSystem->AddSmartTool(pKit.get()))
 					{
 						BuildKitAction(
 							pKit,
 							":/images/icon/scale.png",
 							":/images/icon/scale.png",
 							":/images/icon/scale_press.png",
 							":/images/icon/scale.png"
 							);
 					}
 				}

				///创建红外
				{
					QDesktopWidget* desktopWidget = QApplication::desktop();
					double sH = GetAvailableGeometry().height() * 0.23148;

					osg::ref_ptr<FeKit::CSulIR> pKit = new FeKit::CSulIR(
						m_opSystemService->GetRenderContext(),
						m_opSystemService->GetRenderContext()->GetMapNode(),
						0.25,
						0.25);
					QString strTitle = QString(tr("IR"));
					pKit->SetKey(strTitle.toStdString());
					pKit->SetTitle(strTitle.toStdString());

					if(opSystem->AddSmartTool(pKit.get()))
					{
						BuildKitAction(
							pKit, 
							":/images/icon/ir.png",
							":/images/icon/ir.png",
							":/images/icon/ir_press.png",
							":/images/icon/ir.png"
							);
					}
					pKit->Hide();
				}

				/////创建网格线
				//{
				//	std::string strTitle = QString(tr("Graticule")).toStdString();
				//	osg::ref_ptr<FeKit::FeGraticule> pKit = new FeKit::FeGraticule(
				//		m_opSystemService->GetRenderContext()->GetMapNode(),
				//		m_opSystemService->GetRenderContext()->GetView(), 
				//		m_opSystemService->GetManipulatorManager()->GetFreeEarthManipulator());
				//	pKit->SetKey(strTitle);
				//	pKit->SetTitle(strTitle);
				//	if(opSystem->AddSmartTool(pKit.get()))
				//	{
				//		BuildKitAction(
				//			pKit, 
				//			":/images/icon/网格.png",
				//			":/images/icon/网格.png",
				//			":/images/icon/网格_press.png",
				//			":/images/icon/网格.png"
				//			);
				//	}
				//}

				/////创建相机信息
				//{
				//	std::string strTitle = QString(tr("ButtomInfo")).toStdString();
				//	osg::ref_ptr<FeKit::CBottomInfoBar> pKit = new FeKit::CBottomInfoBar(
				//		m_opSystemService->GetRenderContext(),
				//		0,
				//		0, 
				//		1680,
				//		25, 
				//		FeKit::CPlaceStrategy::RIGHT_BOTTOM);
				//	pKit->SetKey(strTitle);
				//	pKit->SetTitle(strTitle);
				//	
				//	if(opSystem->AddSmartTool(pKit.get()))
				//	{
				//		BuildKitAction(
				//			pKit,
				//			":/images/icon/scene_info.png",
				//			":/images/icon/scene_info.png",
				//			":/images/icon/scene_info_press.png",
				//			":/images/icon/scene_info.png");
				//	}
				//}

				/////屏幕LOGO
				//{
				//	std::string strTitle = QString(tr("MiniLogo")).toStdString();
				//	osg::ref_ptr<FeKit::CMiniLogo> pKit = new FeKit::CMiniLogo(
				//		10,
				//		20, 
				//		200,
				//		60, 
				//		FeKit::CPlaceStrategy::RIGHT_BOTTOM,
				//		m_opSystemService->GetRenderContext()->GetView(),
				//		"FreeEarth Platform");
				//	pKit->SetKey(strTitle);
				//	pKit->SetTitle(strTitle);
				//	pKit->Show();

				//	if(opSystem->AddSmartTool(pKit.get()))
				//	{
				//		BuildKitAction(
				//			pKit, 
				//			":/images/icon/mini_logo.png", 
				//			":/images/icon/mini_logo.png", 
				//			":/images/icon/mini_logo_press.png", 
				//			":/images/icon/mini_logo.png"
				//			);
				//	}
				//}

				//添加工具条分割线
				CFreeToolBar* pToolBar = GetToolBar();
				pToolBar->AddSperator();
			}
		}
	}

	void CViewPluginInterface::BuildKitAction( 
		FeKit::CKitsCtrl* pKitsCtrl,
		const QString& strNormalIcon,
		const QString& strHoverIcon, 
		const QString& strPressIcon,
		const QString& strDisableIcon )
	{
		if(pKitsCtrl)
		{
			QString strTitle = pKitsCtrl->GetTitle().c_str();
			QAction* pToolAction = CreateMenuAndToolAction(
				strTitle,
				strNormalIcon, 
				strHoverIcon, 
				strPressIcon, 
				strDisableIcon, 
				true,
				!pKitsCtrl->IsHide());

			CKitViewUnit* pKitViewUnit = new CKitViewUnit(pKitsCtrl, this);
			connect(pToolAction, SIGNAL(toggled(bool)), pKitViewUnit, SLOT(SlotDoAction(bool)));
		}
	}
}

