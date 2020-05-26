#include <measure/MeasurePluginInterface.h>

#include <mainWindow/FreeMainWindow.h>
#include <mainWindow/FreeToolBar.h>
#include <mainWindow/FreeStatusBar.h>

#include <measure/FillCutAnalysisWidget.h>
#include <measure/ProfileAnalysisWidget.h>
#include <measure/FloodAnalysisWidget.h>
#include <measure/DistanceWidget.h>
#include <measure/BuildingsViewWidget.h>
#include <measure/ContourWidget.h>


namespace FreeViewer
{

	CMeasurePluginInterface::CMeasurePluginInterface( CFreeMainWindow* pMainWindow )
		: CUIObserver(pMainWindow)
		,m_rpSystem(NULL)
		,m_pActiveAction(NULL)
		,m_pSpaceDistAction(NULL)
		,m_pSurfaceDistAction(NULL)
		,m_pHeightDistAction(NULL)
		,m_pSpaceAreaAction(NULL)
		,m_pShadowAreaAction(NULL)
		,m_pLineVisibleAction(NULL)
		,m_pRadialLineVisibleAction(NULL)
		,m_pBuildingsVisibleAction(NULL)
		,m_pProfileAnalysisAction(NULL)
		,m_pGradientAnalysisAction(NULL)
		,m_pFloodAnalysisAction(NULL)
		,m_pFillCutAnalysisAction(NULL)
		,m_pContourAction(NULL)
		, m_pMeasureResultDlg(NULL)
	{
		m_strMenuTitle = QString(tr("Measure"));

		InitWidget();
	}

	CMeasurePluginInterface::~CMeasurePluginInterface()
	{
		if(m_pMeasureResultDlg)
		{
			m_pMeasureResultDlg->close();
		}
	}

	void CMeasurePluginInterface::InitWidget()
	{
		BuildContext();
	}

	void CMeasurePluginInterface::BuildContext()
	{
		if(m_opSystemService.valid() && m_pMainWindow)
		{
			m_rpSystem =  new FeMeasure::CMeasureSys();
		
			if (m_rpSystem->Initialize(m_opSystemService->GetRenderContext()))
			{
				m_pSpaceDistAction = CreateMenuAndToolAction(
					tr("Space Distance"), 
					QString(":/images/icon/space_distance.png"),
					QString(":/images/icon/space_distance.png"),
					QString(":/images/icon/space_distance_press.png"), 
					QString(":/images/icon/space_distance.png"),
					true,
					false);
				connect(m_pSpaceDistAction, SIGNAL(triggered(bool)), this, SLOT(SlotSpaceDistMeasure(bool)));

				m_pSurfaceDistAction = CreateMenuAndToolAction(
					tr("Surface Distance"), 
					QString(":/images/icon/surface_distance.png"),
					QString(":/images/icon/surface_distance.png"),
					QString(":/images/icon/surface_distance_press.png"), 
					QString(":/images/icon/surface_distance.png"),
					true,
					false);
				connect(m_pSurfaceDistAction, SIGNAL(triggered(bool)), this, SLOT(SlotSurfDistMeasure(bool)));

				m_pHeightDistAction = CreateMenuAndToolAction(
					tr("Height Distance"), 
					QString(":/images/icon/height_measure.png"),
					QString(":/images/icon/height_measure.png"),
					QString(":/images/icon/height_measure_press.png"), 
					QString(":/images/icon/height_measure.png"),
					true,
					false);
				connect(m_pHeightDistAction, SIGNAL(triggered(bool)), this, SLOT(SlotHeiDistMeasure(bool)));

				m_pSpaceAreaAction = CreateMenuAndToolAction(
					tr("Space Area"), 
					QString(":/images/icon/area_measure.png"),
					QString(":/images/icon/area_measure.png"),
					QString(":/images/icon/area_measure_press.png"), 
					QString(":/images/icon/area_measure.png"),
					true,
					false);
				connect(m_pSpaceAreaAction, SIGNAL(triggered(bool)), this, SLOT(SlotSpaceAreaMeasure(bool)));

				m_pShadowAreaAction = CreateMenuAndToolAction(
					tr("Shadow Area"), 
					QString(":/images/icon/project_area.png"),
					QString(":/images/icon/project_area.png"),
					QString(":/images/icon/project_area_press.png"), 
					QString(":/images/icon/project_area.png"),
					true,
					false);
				connect(m_pShadowAreaAction, SIGNAL(triggered(bool)), this, SLOT(SlotShadowAreaMeasure(bool)));

				m_pLineVisibleAction = CreateMenuAndToolAction(
					tr("Line Visible"), 
					QString(":/images/icon/line_visible_measure.png"),
					QString(":/images/icon/line_visible_measure.png"),
					QString(":/images/icon/line_visible_measure_press.png"), 
					QString(":/images/icon/line_visible_measure.png"),
					true,
					false);
				connect(m_pLineVisibleAction, SIGNAL(triggered(bool)), this, SLOT(SlotLineVisibleMeasure(bool)));

				m_pRadialLineVisibleAction = CreateMenuAndToolAction(
					tr("Radial Line Visible"), 
					QString(":/images/icon/view_measure.png"),
					QString(":/images/icon/view_measure.png"),
					QString(":/images/icon/view_measure_press.png"), 
					QString(":/images/icon/view_measure.png"),
					true,
					false);
			    connect(m_pRadialLineVisibleAction, SIGNAL(triggered(bool)), this, SLOT(SlotRadialLineVisibleMeasure(bool)));

				//小版本集成
				//m_pBuildingsVisibleAction = CreateMenuAndToolAction(
				//	tr("Buildings Visible"), 
				//	QString(":/images/icon/building_view.png"),
				//	QString(":/images/icon/building_view.png"),
				//	QString(":/images/icon/building_view_press.png"), 
				//	QString(":/images/icon/building_view.png"),
				//	true,
				//	false);
				//connect(m_pBuildingsVisibleAction, SIGNAL(triggered(bool)), this, SLOT(SlotBuildingVisibleMeasure(bool)));

				m_pProfileAnalysisAction = CreateMenuAndToolAction(
					tr("Profile Analysis"), 
					QString(":/images/icon/profile_measure.png"),
					QString(":/images/icon/profile_measure.png"),
					QString(":/images/icon/profile_measure_press.png"), 
					QString(":/images/icon/profile_measure.png"),
					true,
					false);
				connect(m_pProfileAnalysisAction, SIGNAL(triggered(bool)), this, SLOT(SlotProfileMeasure(bool)));

				m_pGradientAnalysisAction = CreateMenuAndToolAction(
					tr("Gradient Analysis"), 
					QString(":/images/icon/gradient_measure.png"),
					QString(":/images/icon/gradient_measure.png"),
					QString(":/images/icon/gradient_measure_press.png"), 
					QString(":/images/icon/gradient_measure.png"),
					true,
					false);
				connect(m_pGradientAnalysisAction, SIGNAL(triggered(bool)), this, SLOT(SlotGradientMeasure(bool)));

				m_pFloodAnalysisAction = CreateMenuAndToolAction(
					tr("Flood Analysis"), 
					QString(":/images/icon/flood_measure.png"),
					QString(":/images/icon/flood_measure.png"),
					QString(":/images/icon/flood_measure_press.png"), 
					QString(":/images/icon/flood_measure.png"),
					true,
					false);
				connect(m_pFloodAnalysisAction, SIGNAL(triggered(bool)), this, SLOT(SlotFloodMeasure(bool)));

				m_pFillCutAnalysisAction = CreateMenuAndToolAction(
					tr("FillCut Analysis"), 
					QString(":/images/icon/cutfill_measure.png"),
					QString(":/images/icon/cutfill_measure.png"),
					QString(":/images/icon/cutfill_measure_press.png"), 
					QString(":/images/icon/cutfill_measure.png"),
					true,
					false);
				connect(m_pFillCutAnalysisAction, SIGNAL(triggered(bool)), this, SLOT(SlotFillCutMeasure(bool)));

				m_pContourAction = CreateMenuAndToolAction(
					tr("Contour Measure"), 
					QString(":/images/icon/contour_measure.png"),
					QString(":/images/icon/contour_measure.png"),
					QString(":/images/icon/contour_measure_press.png"), 
					QString(":/images/icon/contour_measure.png"),
					true,
					false);
				connect(m_pContourAction, SIGNAL(triggered(bool)), this, SLOT(SlotContourMeasure(bool)));

				m_pDeleteAction = CreateMenuAndToolAction(
					tr("Clear Measure"), 
					QString(":/images/icon/clear_mark.png"),
					QString(":/images/icon/clear_mark.png"),
					QString(":/images/icon/clear_mark_press.png"), 
					QString(":/images/icon/clear_mark.png"),
					false,
					false);
				connect(m_pDeleteAction, SIGNAL(triggered(bool)), this, SLOT(SlotDeleteMeasure(bool)));

				//添加工具条分割线
				CFreeToolBar* pToolBar = GetToolBar();
				pToolBar->AddSperator();
			}
		}
	}

	void CMeasurePluginInterface::SetMutex(QAction* pCurrentAction)
	{
		if(m_pActiveAction)
		{
			m_pActiveAction->setChecked(false);
		}

		m_pActiveAction = pCurrentAction;

		// 状态栏操作提示
		if(m_pMainWindow && m_pMainWindow->GetStatusBar())
		{
			m_pMainWindow->GetStatusBar()->SetMessageText(m_pActiveAction ?
				QString(tr("Tips: click the left mouse button to add point, and right button to end measure")) : "");
		}
	}

	void CMeasurePluginInterface::CloseLastMeasureWidget()
	{
		if(m_pMeasureResultDlg)
		{
			m_pMeasureResultDlg->close();
			delete m_pMeasureResultDlg;
			m_pMeasureResultDlg = NULL;
		}
	}

	void CMeasurePluginInterface::CreateAndBeginMeasure(FeMeasure::IMeasureFactory* factory, 
		CMeasureDisplayWidget* pWidget, QString title, QAction* pToolAction)
	{
		m_pMainWindow->ClearMark();
		m_pMainWindow->ClearPlot();

		if(!factory || !pWidget) return;

		if(m_pMeasureResultDlg != pWidget)
		{
			CloseLastMeasureWidget();
			m_pMeasureResultDlg = pWidget; 
		}
		
		m_pMeasureResultDlg->SetTitleText(title);
		
		if(FeMeasure::CMeasure* pMeasure = factory->Create()) 
		{
			m_rpFactory = factory;
			m_rpSystem->ActiveMeasure(pMeasure);
			pMeasure->RegisterObserver(m_pMeasureResultDlg);
		}

		SetMutex(pToolAction);
	}

	void CMeasurePluginInterface::SlotSpaceDistMeasure(bool bAction)
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CSpaceDistFactory(m_opSystemService->GetRenderContext()), 
				new CMDistanceWidget(m_pMainWindow), tr("Space Distance"), m_pSpaceDistAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotSurfDistMeasure(bool bAction)
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CSurfaceDistFactory(m_opSystemService->GetRenderContext()), 
				new CMDistanceWidget(m_pMainWindow), tr("Surface Distance"), m_pSurfaceDistAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotHeiDistMeasure(bool bAction)
	{
		if(bAction)
		{
			CMDistanceWidget *pDistanceWidget = new CMDistanceWidget(m_pMainWindow);
			pDistanceWidget->setLabelValueUI(tr("Height Distance Value"));
			CreateAndBeginMeasure(new FeMeasure::CHeiDistFactory(m_opSystemService->GetRenderContext()), 
				pDistanceWidget, tr("Height Distance"), m_pHeightDistAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotSpaceAreaMeasure(bool bAction)
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CSpaceAreaFactory(m_opSystemService->GetRenderContext()), 
				new CMAreaWidget(m_pMainWindow), tr("Space Area"), m_pSpaceAreaAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotShadowAreaMeasure(bool bAction)
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CShadowAreaFactory(m_opSystemService->GetRenderContext()), 
				new CMAreaWidget(m_pMainWindow), tr("Shadow Area"), m_pShadowAreaAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotLineVisibleMeasure(bool bAction)
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CIntervisibleFactory(m_opSystemService->GetRenderContext()), 
				new CMVisibleLineWidget(m_pMainWindow), tr("Line Visible"), m_pLineVisibleAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotRadialLineVisibleMeasure(bool bAction)
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CRadialLineFactory(m_opSystemService->GetRenderContext()), 
				new CMRadialLineWidget(m_pMainWindow), tr("Radial Line Visible"), m_pRadialLineVisibleAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotBuildingVisibleMeasure( bool bAction )
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CBuildingViewFactory(m_opSystemService->GetRenderContext()), 
				new CMBuildingsViewWidget(m_pMainWindow), tr("Buildings Visible"), m_pBuildingsVisibleAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotFloodMeasure(bool bAction)
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CFloodFactory(m_opSystemService->GetRenderContext()), 
				new CMFloodAnalysisWidget(m_pMainWindow), tr("Flood Analysis"), m_pFloodAnalysisAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotProfileMeasure( bool bAction )
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CProfileFactory(m_opSystemService->GetRenderContext()), 
				new CMProfileAnalysisWidget(m_pMainWindow), tr("Profile Analysis"), m_pProfileAnalysisAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotGradientMeasure( bool bAction )
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CGradientFactory(m_opSystemService->GetRenderContext()), 
				new CMGradientWidget(m_pMainWindow), tr("Gradient Analysis"), m_pGradientAnalysisAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotFillCutMeasure( bool bAction )
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CFillCutFactory(m_opSystemService->GetRenderContext()), 
				new CMFillCutAnalysisWidget(m_pMainWindow), tr("FillCut Analysis"), m_pFillCutAnalysisAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

	void CMeasurePluginInterface::SlotDeleteMeasure(bool bAction)
	{
		if(m_rpSystem.valid())
		{
			m_rpSystem->DeactiveMeasure();
			SetMutex(NULL);
		}

		CloseLastMeasureWidget();
	}

	void CMeasurePluginInterface::SlotContourMeasure( bool bAction )
	{
		if(bAction)
		{
			CreateAndBeginMeasure(new FeMeasure::CContourFactory(m_opSystemService->GetRenderContext()), 
				new CContourWidget(m_pMainWindow), tr("Contour Measure"), m_pContourAction);
		}
		else
		{
			SlotDeleteMeasure(true);
		}
	}

}
