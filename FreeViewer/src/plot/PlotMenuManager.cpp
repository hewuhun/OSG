#include <plot/PlotMenuManager.h>

#include <FeUtils/CoordConverter.h>
#include <FeExtNode/ExternNode.h>
#include <FePlots/StraightArrow.h>
#include <FePlots/StraightMoreArrow.h>
#include <FePlots/DovetailDiagonalArrow.h>
#include <FePlots/DovetailDiagonalMoreArrow.h>
#include <FePlots/DoveTailStraightArrow.h>
#include <FePlots/DoveTailStraightMoreArrow.h>
#include <FePlots/DiagonalArrow.h>
#include <FePlots/DiagonalMoreArrow.h>
#include <FePlots/DoubleArrow.h>
//#include <FePlots/RectFlag.h>
//#include <FePlots/CurveFlag.h>
//#include <FePlots/TriangleFlag.h>
#include <FePlots/RoundedRect.h>
#include <FePlots/GatheringPlace.h>
#include <FePlots/CloseCurve.h>
#include <FePlots/BezierCurveArrow.h>
#include <FePlots/PolylineArrow.h>
#include <FePlots/CardinalCurveArrow.h>
#include <FePlots/SectorSearch.h>
#include <FePlots/ParallelSearch.h>
#include <FePlots/StraightLineArrow.h>
//#include <FePlots/Freeline.h>
//#include <FePlots/FreePolygon.h>
#include <plot/PlotPluginInterface.h>
#include <mainWindow/FreeMainWindow.h>
#include <mainWindow/FreeStatusBar.h>

namespace FreeViewer
{
	CPlotMenuManager::CPlotMenuManager(CPlotPluginInterface* pPlotPlugin)
		: m_pPlotPlugin(pPlotPlugin)
		, m_pFolderAction(NULL)
		, m_pActiveAction(NULL)
		, m_pOpenPropertyAction(NULL)
		, m_pDeletePlotAction(NULL)
		, m_pClearPlotAction(NULL)
		, m_pStraightArrowAction(NULL)
		, m_pDovetailDiagonalArrowAction(NULL)
		, m_pDovetailDiagonalMoreArrowAction(NULL)
		, m_pDoveTailStraightArrowAction(NULL)
		, m_pDoveTailStraightMoreArrowAction(NULL)
		, m_pDiagonalArrowAction(NULL)
		, m_pDiagonalMoreArrowAction(NULL)
		, m_pDoubleArrowAction(NULL)
		, m_pRoundedRectAction(NULL)
		, m_pCloseCurveAction(NULL)
		//, m_pRectFlagAction(NULL)
		//, m_pCurveFlagAction(NULL)
		//, m_pTriangleFlagAction(NULL)
		, m_pGatheringPlaceAction(NULL)
		, m_pBezierCurveArrowAction(NULL)
		, m_pPolyLineArrowAction(NULL)
		, m_pSectorSearchAction(NULL)
		, m_pParallelSearchAction(NULL)
		, m_pCardinalCurveArrowAction(NULL)
		, m_pStraightMoreArrowAction(NULL)
		, m_pStraightLineArrowAction(NULL)
		//, m_pFreeLineAction(NULL)
		//, m_pFreePolygonAction(NULL)
	{
	}

	void CPlotMenuManager::InitMenuAndActions()
	{
		if(!m_pPlotPlugin)
		{
			return;
		}

		/// 创建右键菜单关联的动作
		m_pFolderAction = new QAction(tr("Folder"), this);
		m_pFolderAction->setToolTip(tr("Folder"));
		m_pFolderAction->setDisabled(false);
		m_pFolderAction->setCheckable(true);
		connect(m_pFolderAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddFolder(bool)));

		m_pOpenPropertyAction = new QAction(tr("Property"), this);
		m_pOpenPropertyAction->setToolTip(tr("Property"));
		m_pOpenPropertyAction->setDisabled(false);
		connect(m_pOpenPropertyAction, SIGNAL(triggered()), this, SIGNAL(SignalOpenPropertyWidget()));
		
		m_pDeletePlotAction = new QAction(tr("Delete"), this);
		m_pDeletePlotAction->setToolTip(tr("Delete"));
		m_pDeletePlotAction->setDisabled(false);
		connect(m_pDeletePlotAction, SIGNAL(triggered()), this, SIGNAL(SignalDeletePlot()));

		m_pClearPlotAction = new QAction(tr("Clear"), this);
		m_pClearPlotAction->setToolTip(tr("Clear"));
		m_pClearPlotAction->setDisabled(false);
		connect(m_pClearPlotAction, SIGNAL(triggered()), this, SIGNAL(SignalClearPlot()));

		m_pStraightArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("StraightArrow"), 
			QString(":/images/icon/StraightArrow.png"),
			QString(":/images/icon/StraightArrow.png"),
			QString(":/images/icon/StraightArrow_press.png"), 
			QString(":/images/icon/StraightArrow.png"),
			true,
			false);
		connect(m_pStraightArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddStraightArrow(bool)));
		
		m_pStraightMoreArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("StraightMoreArrow"), 
			QString(":/images/icon/StraightMoreArrow.png"),
			QString(":/images/icon/StraightMoreArrow.png"),
			QString(":/images/icon/StraightMoreArrow_press.png"), 
			QString(":/images/icon/StraightMoreArrow.png"),
			true,
			false);
		connect(m_pStraightMoreArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddStraightMoreArrow(bool)));

		m_pDovetailDiagonalArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("DovetailDiagonalArrow"), 
			QString(":/images/icon/DovetailDiagonalArrow.png"),
			QString(":/images/icon/DovetailDiagonalArrow.png"),
			QString(":/images/icon/DovetailDiagonalArrow_press.png"), 
			QString(":/images/icon/DovetailDiagonalArrow.png"),
			true,
			false);
		connect(m_pDovetailDiagonalArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddDovetailDiagonalArrow(bool)));

		m_pDovetailDiagonalMoreArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("DovetailDiagonalMoreArrow"), 
			QString(":/images/icon/DovetailDiagonalMoreArrow.png"),
			QString(":/images/icon/DovetailDiagonalMoreArrow.png"),
			QString(":/images/icon/DovetailDiagonalMoreArrow_press.png"), 
			QString(":/images/icon/DovetailDiagonalMoreArrow.png"),
			true,
			false);
		connect(m_pDovetailDiagonalMoreArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddDovetailDiagonalMoreArrow(bool)));

		m_pDoveTailStraightArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("DoveTailStraightArrow"), 
			QString(":/images/icon/DoveTailStraightArrow.png"),
			QString(":/images/icon/DoveTailStraightArrow.png"),
			QString(":/images/icon/DoveTailStraightArrow_press.png"), 
			QString(":/images/icon/DoveTailStraightArrow.png"),
			true,
			false);
		connect(m_pDoveTailStraightArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddDoveTailStraightArrow(bool)));

		m_pDoveTailStraightMoreArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("DoveTailStraightMoreArrow"), 
			QString(":/images/icon/DoveTailStraightMoreArrow.png"),
			QString(":/images/icon/DoveTailStraightMoreArrow.png"),
			QString(":/images/icon/DoveTailStraightMoreArrow_press.png"), 
			QString(":/images/icon/DoveTailStraightMoreArrow.png"),
			true,
			false);
		connect(m_pDoveTailStraightMoreArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddDoveTailStraightMoreArrow(bool)));

		m_pDiagonalArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("DiagonalArrow"), 
			QString(":/images/icon/DiagonalArrow.png"),
			QString(":/images/icon/DiagonalArrow.png"),
			QString(":/images/icon/DiagonalArrow_press.png"), 
			QString(":/images/icon/DiagonalArrow.png"),
			true,
			false);
		connect(m_pDiagonalArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddDiagonalArrow(bool)));

		m_pDiagonalMoreArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("DiagonalMoreArrow"), 
			QString(":/images/icon/DiagonalMoreArrow.png"),
			QString(":/images/icon/DiagonalMoreArrow.png"),
			QString(":/images/icon/DiagonalMoreArrow_press.png"), 
			QString(":/images/icon/DiagonalMoreArrow.png"),
			true,
			false);
		connect(m_pDiagonalMoreArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddDiagonalMoreArrow(bool)));

		m_pDoubleArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("DoubleArrow"), 
			QString(":/images/icon/DoubleArrow.png"),
			QString(":/images/icon/DoubleArrow.png"),
			QString(":/images/icon/DoubleArrow_press.png"), 
			QString(":/images/icon/DoubleArrow.png"),
			true,
			false);
		connect(m_pDoubleArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddDoubleArrow(bool)));

		m_pCloseCurveAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("CloseCurve"), 
			QString(":/images/icon/CloseCurve.png"),
			QString(":/images/icon/CloseCurve.png"),
			QString(":/images/icon/CloseCurve_press.png"), 
			QString(":/images/icon/CloseCurve.png"),
			true,
			false);
		connect(m_pCloseCurveAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddCloseCurve(bool)));

		m_pGatheringPlaceAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("GatheringPlace"), 
			QString(":/images/icon/GatheringPlace.png"),
			QString(":/images/icon/GatheringPlace.png"),
			QString(":/images/icon/GatheringPlace_press.png"), 
			QString(":/images/icon/GatheringPlace.png"),
			true,
			false);
		connect(m_pGatheringPlaceAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddGatheringPlace(bool)));

		/*
		m_pRectFlagAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("RectFlag"), 
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack_press.png"), 
			QString(":/images/icon/attack.png"),
			true,
			false);
		connect(m_pRectFlagAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddRectFlag(bool)));

		m_pCurveFlagAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("CurveFlag"), 
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack_press.png"), 
			QString(":/images/icon/attack.png"),
			true,
			false);
		connect(m_pCurveFlagAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddCurveFlag(bool)));

		m_pTriangleFlagAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("TriangleFlag"), 
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack_press.png"), 
			QString(":/images/icon/attack.png"),
			true,
			false);
		connect(m_pTriangleFlagAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddTriangleFlag(bool)));
		*/

		m_pRoundedRectAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("RoundedRect"), 
			QString(":/images/icon/RoundedRect.png"),
			QString(":/images/icon/RoundedRect.png"),
			QString(":/images/icon/RoundedRect_press.png"), 
			QString(":/images/icon/RoundedRect.png"),
			true,
			false);
		connect(m_pRoundedRectAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddRoundedRect(bool)));

		m_pBezierCurveArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("BezierCurveArrow"), 
			QString(":/images/icon/BezierCurveArrow.png"),
			QString(":/images/icon/BezierCurveArrow.png"),
			QString(":/images/icon/BezierCurveArrow_press.png"), 
			QString(":/images/icon/BezierCurveArrow.png"),
			true,
			false);
		connect(m_pBezierCurveArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddBezierCurveArrow(bool)));

		m_pPolyLineArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("PolyLineArrow"), 
			QString(":/images/icon/PolyLineArrow.png"),
			QString(":/images/icon/PolyLineArrow.png"),
			QString(":/images/icon/PolyLineArrow_press.png"), 
			QString(":/images/icon/PolyLineArrow.png"),
			true,
			false);
		connect(m_pPolyLineArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddPolyLineArrow(bool)));

		m_pParallelSearchAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("ParallelSearch"), 
			QString(":/images/icon/ParallelSearch.png"),
			QString(":/images/icon/ParallelSearch.png"),
			QString(":/images/icon/ParallelSearch_press.png"), 
			QString(":/images/icon/ParallelSearch.png"),
			true,
			false);
		connect(m_pParallelSearchAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddParallelSearch(bool)));

		m_pSectorSearchAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("SectorSearch"), 
			QString(":/images/icon/SectorSearch.png"),
			QString(":/images/icon/SectorSearch.png"),
			QString(":/images/icon/SectorSearch_press.png"), 
			QString(":/images/icon/SectorSearch.png"),
			true,
			false);
		connect(m_pSectorSearchAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddSectorSearch(bool)));

		m_pCardinalCurveArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("CardinalCurveArrow"), 
			QString(":/images/icon/CardinalCurveArrow.png"),
			QString(":/images/icon/CardinalCurveArrow.png"),
			QString(":/images/icon/CardinalCurveArrow_press.png"), 
			QString(":/images/icon/CardinalCurveArrow.png"),
			true,
			false);
		connect(m_pCardinalCurveArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddCardinalCurveArrow(bool)));

		m_pStraightLineArrowAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("StraightLineArrow"), 
			QString(":/images/icon/StraightLineArrow.png"),
			QString(":/images/icon/StraightLineArrow.png"),
			QString(":/images/icon/StraightLineArrow_press.png"), 
			QString(":/images/icon/StraightLineArrow.png"),
			true,
			false);
		connect(m_pStraightLineArrowAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddStraightLineArrow(bool)));

		/*
		m_pFreeLineAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("FreeLine"), 
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack_press.png"), 
			QString(":/images/icon/attack.png"),
			true,
			false);
		connect(m_pFreeLineAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddFreeLine(bool)));

		m_pFreePolygonAction = m_pPlotPlugin->CreateMenuAndToolAction(
			tr("FreePolygon"), 
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack_press.png"), 
			QString(":/images/icon/attack.png"),
			true,
			false);
		connect(m_pFreePolygonAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddFreePolygon(bool)));
		*/
	}

	QMenu* CPlotMenuManager::CreatePopMenu( FeExtNode::CExternNode* pExternNode, QWidget* pMenuParent )
	{
		if(!m_pPlotPlugin)
		{
			return NULL;
		}

		QMenu* pPopMenu = new QMenu(pMenuParent);
		
		pPopMenu->addMenu(CreateAddMenu(pMenuParent));

		if (pExternNode && pExternNode->AsComposeNode())
		{
			pPopMenu->addAction(m_pClearPlotAction);

			if(pExternNode != m_pPlotPlugin->GetMarkSys()->GetRootMark())
			{
				pPopMenu->addAction(m_pDeletePlotAction);
			}
		}
		else
		{
			pPopMenu->addAction(m_pDeletePlotAction);
		}

		pPopMenu->addAction(m_pOpenPropertyAction);

		return pPopMenu;
	}

	QMenu* CPlotMenuManager::CreateRootPopMenu( FeExtNode::CExternNode* pExternNode, QWidget* pMenuParent )
	{
		if(!m_pPlotPlugin)
		{
			return NULL;
		}

		QMenu* pPopMenu = new QMenu(pMenuParent);

		if (pExternNode && pExternNode->AsComposeNode())
		{
			m_pFolderAction->setText(tr("Add Folder"));
			pPopMenu->addAction(m_pFolderAction);
			pPopMenu->addAction(m_pClearPlotAction);
		}

		return pPopMenu;
	}

	QMenu* CPlotMenuManager::CreateAddMenu(QWidget* pMenuParent)
	{
		QMenu* pAddLayerMenu = new QMenu(tr("Add Plot"), pMenuParent);
		m_pFolderAction->setText(tr("Folder"));
		pAddLayerMenu->addAction(m_pFolderAction);
		pAddLayerMenu->addAction(m_pStraightArrowAction);
		pAddLayerMenu->addAction(m_pStraightMoreArrowAction);
		pAddLayerMenu->addAction(m_pDovetailDiagonalArrowAction);
		pAddLayerMenu->addAction(m_pDovetailDiagonalMoreArrowAction);
		pAddLayerMenu->addAction(m_pDoveTailStraightArrowAction);
		pAddLayerMenu->addAction(m_pDoveTailStraightMoreArrowAction);
		pAddLayerMenu->addAction(m_pDiagonalArrowAction);
		pAddLayerMenu->addAction(m_pDiagonalMoreArrowAction);
		pAddLayerMenu->addAction(m_pDoubleArrowAction);
		pAddLayerMenu->addAction(m_pCloseCurveAction);
		//pAddLayerMenu->addAction(m_pCurveFlagAction);
		//pAddLayerMenu->addAction(m_pRectFlagAction);
		//pAddLayerMenu->addAction(m_pTriangleFlagAction);
		pAddLayerMenu->addAction(m_pRoundedRectAction);
		pAddLayerMenu->addAction(m_pStraightLineArrowAction);
		pAddLayerMenu->addAction(m_pBezierCurveArrowAction);
		pAddLayerMenu->addAction(m_pPolyLineArrowAction);
		pAddLayerMenu->addAction(m_pParallelSearchAction);
		pAddLayerMenu->addAction(m_pSectorSearchAction);
		pAddLayerMenu->addAction(m_pCardinalCurveArrowAction);
		//pAddLayerMenu->addAction(m_pFreeLineAction);
		//pAddLayerMenu->addAction(m_pFreePolygonAction);
		return pAddLayerMenu;
	}
	
	void CPlotMenuManager::HandlePlotAction( osg::ref_ptr<FeExtNode::CExternNode> pNode, QAction* pAction, bool bAction )
	{
		if(!m_pPlotPlugin)
		{
			return;
		}

		//清除测量和标记，防止鼠标事件冲突
		m_pPlotPlugin->GetMainWindow()->ClearMeasure();
		m_pPlotPlugin->GetMainWindow()->ClearMark();

		if(!bAction)
		{
			m_pPlotPlugin->StopEventCapture();
			m_pPlotPlugin->FailedToDraw();
			SetMutex(NULL);
		}
		else
		{
			if(m_pPlotPlugin->IsDrawingOrEditing())
			{
				if(pAction) pAction->setChecked(false);
				m_pPlotPlugin->FailedToDraw();
				return;
			}
		
			if(pNode)
			{
				m_pPlotPlugin->GetPlotProcessor().DoProcess(pNode, CPlotVisitProcessor::E_PLOT_INITIALIZE);
				m_pPlotPlugin->SetActivePlotNode(pNode);
				SetMutex(pAction);
				m_pPlotPlugin->StartEventCapture();
			}
		}
	}

	void CPlotMenuManager::SlotAddFolder( bool bAction )
	{
		if(bAction && m_pPlotPlugin)
		{
			//清除测量和标记，防止鼠标事件冲突
			m_pPlotPlugin->GetMainWindow()->ClearMeasure();
			m_pPlotPlugin->GetMainWindow()->ClearMark();

			if(m_pFolderAction) m_pFolderAction->setChecked(false);

			if(m_pPlotPlugin->IsDrawingOrEditing())
			{
				m_pPlotPlugin->FailedToDraw();
				return;
			}

			FeExtNode::CExternNode* pNode = new FeExtNode::CExComposeNode(new FeExtNode::CExComposeNodeOption());
			m_pPlotPlugin->GetPlotProcessor().DoProcess(pNode, CPlotVisitProcessor::E_PLOT_INITIALIZE);

			m_pPlotPlugin->SetActivePlotNode(pNode);
			m_pPlotPlugin->StartDrawPlot();
			m_pPlotPlugin->SuccessToDraw();
		}
	}

	void CPlotMenuManager::SlotAddStraightArrow(bool bAction)
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CStraightArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CStraightArrowOption()) 
				: NULL, 
				m_pStraightArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddDovetailDiagonalArrow(bool bAction)
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CDovetailDiagonalArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CDovetailDiagonalArrowOption()) 
				: NULL, 
				m_pDovetailDiagonalArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddDovetailDiagonalMoreArrow( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CDovetailDiagonalMoreArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CDovetailDiagonalMoreArrowOption()) 
				: NULL, 
				m_pDovetailDiagonalMoreArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SetMutex( QAction* pCurrentAction )
	{
		if(m_pActiveAction)
		{
			m_pActiveAction->setChecked(false);
		}

		m_pActiveAction = pCurrentAction;

		// 状态栏操作提示
		CFreeStatusBar *pStatusBar = m_pPlotPlugin->GetMainWindow()->GetStatusBar();
		if(pStatusBar)
		{
			pStatusBar->SetMessageText(m_pActiveAction ?
				QString(tr("Tips: click the left mouse button to add point, and right button to end plot")) : "");
		}
	}

	void CPlotMenuManager::SlotAddDoveTailStraightArrow( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CDoveTailStraightArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CDoveTailStraightArrowOption()) 
				: NULL, 
				m_pDoveTailStraightArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddDoveTailStraightMoreArrow( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CDoveTailStraightMoreArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CDoveTailStraightMoreArrowOption()) 
				: NULL, 
				m_pDoveTailStraightMoreArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddDiagonalArrow( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CDiagonalArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CDiagonalArrowOption()) 
				: NULL, 
				m_pDiagonalArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddDiagonalMoreArrow( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CDiagonalMoreArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CDiagonalMoreArrowOption()) 
				: NULL, 
				m_pDiagonalMoreArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddDoubleArrow( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CDoubleArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CDoubleArrowOption()) 
				: NULL, 
				m_pDoubleArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddCloseCurve( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CCloseCurve(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CCloseCurveOption()) 
				: NULL, 
				m_pCloseCurveAction, bAction);
		}
	}

	/*
	void CPlotMenuManager::SlotAddRectFlag( bool bAction )                       
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CRectFlag(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CRectFlagOption()) 
				: NULL, 
				m_pRectFlagAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddCurveFlag( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CCurveFlag(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CCurveFlagOption()) 
				: NULL, 
				m_pCurveFlagAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddTriangleFlag( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CTriangleFlag(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CTriangleFlagOption()) 
				: NULL, 
				m_pTriangleFlagAction, bAction);
		}

	}
	*/

	void CPlotMenuManager::SlotAddRoundedRect( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CRoundedRect(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CRoundedRectOption()) 
				: NULL, 
				m_pRoundedRectAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddGatheringPlace( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CGatheringPlace(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CGatheringPlaceOption()) 
				: NULL, 
				m_pGatheringPlaceAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddBezierCurveArrow( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CBezierCurveArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CBezierCurveArrowOption()) 
				: NULL, 
				m_pBezierCurveArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddPolyLineArrow( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CPolyLineArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CPolyLineArrowOption()) 
				: NULL, 
				m_pPolyLineArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddParallelSearch( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CParallelSearch(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CParallelSearchOption()) 
				: NULL, 
				m_pParallelSearchAction, bAction);
		}

	}

	void CPlotMenuManager::SlotAddSectorSearch( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CSectorSearch(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CSectorSearchOption()) 
				: NULL, 
				m_pSectorSearchAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddCardinalCurveArrow( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CCardinalCurveArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CCardinalCurveArrowOption()) 
				: NULL, 
				m_pCardinalCurveArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddStraightMoreArrow( bool bAction )
	{

		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CStraightMoreArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CStraightMoreArrowOption()) 
				: NULL, 
				m_pStraightMoreArrowAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddStraightLineArrow( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CStraightLineArrow(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CStraightLineArrowOption()) 
				: NULL, 
				m_pStraightLineArrowAction, bAction);
		}
	}

	/*
	void CPlotMenuManager::SlotAddFreeLine( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CFreeLine(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CFreeLineOption()) 
				: NULL, 
				m_pFreeLineAction, bAction);
		}
	}

	void CPlotMenuManager::SlotAddFreePolygon( bool bAction )
	{
		if(m_pPlotPlugin)
		{
			HandlePlotAction( (bAction && !m_pPlotPlugin->IsDrawingOrEditing()) ? 
				new FePlots::CFreePolygon(m_pPlotPlugin->GetSystemService()->GetRenderContext(), new FePlots::CFreePolygonOption()) 
				: NULL, 
				m_pFreePolygonAction, bAction);
		}
	}
	*/

}


