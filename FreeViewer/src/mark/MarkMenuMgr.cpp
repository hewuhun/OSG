#include <mark/MarkMenuMgr.h>

#include <FeUtils/CoordConverter.h>
#include <FeExtNode/ExLineNode.h>
#include <FeExtNode/ExternNode.h>
#include <FeExtNode/ExLodModelNode.h>
#include <FeExtNode/ExPlaceNode.h>
#include <FeExtNode/ExPolygonNode.h>
#include <FeExtNode/ExSectorNode.h>
#include <FeExtNode/ExRectNode.h>
#include <FeExtNode/ExEllipseNode.h>
#include <FeExtNode/ExCircleNode.h>
#include <FeExtNode/ExTextNode.h>
#include <FeExtNode/ExTiltModelNode.h>
#include <FeExtNode/ExLabelNode.h>
//#include <FeExtNode/ExStraightArrowNode.h>
//#include <FeExtNode/ExAssaultNode.h>
//#include <FeExtNode/ExAttackNode.h>
#include <FeExtNode/ExBillBoardNode.h>
#include <FeExtNode/ExOverLayNode.h>
#include <FeExtNode/ExParticleNode.h>

#include <mark/MarkPluginInterface.h>
#include <mainWindow/FreeMainWindow.h>
#include <mainWindow/FreeStatusBar.h>

namespace FreeViewer
{
	CMarkMenuMgr::CMarkMenuMgr(CMarkPluginInterface* pMarkPlugin)
		: m_pMarkPlugin(pMarkPlugin)
		, m_pFolderAction(NULL)
		, m_pPointAction(NULL)
		, m_pLineAction(NULL)
		, m_pPolygonAction(NULL)
		//, m_pStraightAction(NULL)
		//, m_pAssaultAction(NULL)
		//, m_pAttackAction(NULL)
		, m_pArcAction(NULL)
		, m_pEllipseAction(NULL)
		, m_pRectAction(NULL)
		, m_pSectorAction(NULL)
		, m_pCircleAction(NULL)
		, m_pTextAction(NULL)
		, m_pLabelAction(NULL)
		, m_pModelAction(NULL)
		, m_pTiltModelAction(NULL)
		, m_pBillboardAction(NULL)
		, m_pOverlayAction(NULL)
		, m_pActiveAction(NULL)
		, m_pParticleAction(NULL)
	{
	}

	void CMarkMenuMgr::InitMenuAndActions()
	{
		if(!m_pMarkPlugin) return;

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
		
		m_pDeleteMarkAction = new QAction(tr("Delete"), this);
		m_pDeleteMarkAction->setToolTip(tr("Delete"));
		m_pDeleteMarkAction->setDisabled(false);
		connect(m_pDeleteMarkAction, SIGNAL(triggered()), this, SIGNAL(SignalDeleteMark()));

		m_pClearMarkAction = new QAction(tr("Clear"), this);
		m_pClearMarkAction->setToolTip(tr("Clear"));
		m_pClearMarkAction->setDisabled(false);
		connect(m_pClearMarkAction, SIGNAL(triggered()), this, SIGNAL(SignalClearMark()));

		m_pPointAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Point"), 
			QString(":/images/icon/point.png"),
			QString(":/images/icon/point.png"),
			QString(":/images/icon/point_press.png"), 
			QString(":/images/icon/point.png"),
			true,
			false);
		connect(m_pPointAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddPoint(bool)));

		m_pLineAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Line"), 
			QString(":/images/icon/line.png"),
			QString(":/images/icon/line.png"),
			QString(":/images/icon/line_press.png"), 
			QString(":/images/icon/line.png"),
			true,
			false);
		connect(m_pLineAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddLine(bool)));

		m_pPolygonAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Polygon"), 
			QString(":/images/icon/polygon.png"),
			QString(":/images/icon/polygon.png"),
			QString(":/images/icon/polygon_press.png"), 
			QString(":/images/icon/polygon.png"),
			true,
			false);
		connect(m_pPolygonAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddPolygon(bool)));

		/*
		m_pStraightAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Straight"), 
			QString(":/images/icon/direct_arrow.png"),
			QString(":/images/icon/direct_arrow.png"),
			QString(":/images/icon/direct_arrow_press.png"), 
			QString(":/images/icon/direct_arrow.png"),
			true,
			false);
		connect(m_pStraightAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddStraight(bool)));

		m_pAssaultAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Assault"), 
			QString(":/images/icon/assault.png"),
			QString(":/images/icon/assault.png"),
			QString(":/images/icon/assault_press.png"), 
			QString(":/images/icon/assault.png"),
			true,
			false);
		connect(m_pAssaultAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddAssault(bool)));

		m_pAttackAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Attack"), 
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack.png"),
			QString(":/images/icon/attack_press.png"), 
			QString(":/images/icon/attack.png"),
			true,
			false);
		connect(m_pAttackAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddAttack(bool)));
		*/
		m_pArcAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Arc"), 
			QString(":/images/icon/arc.png"),
			QString(":/images/icon/arc.png"),
			QString(":/images/icon/arc_press.png"), 
			QString(":/images/icon/arc.png"),
			true,
			false);
		connect(m_pArcAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddArc(bool)));

		m_pEllipseAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Ellipse"), 
			QString(":/images/icon/ellipse.png"),
			QString(":/images/icon/ellipse.png"),
			QString(":/images/icon/ellipse_press.png"), 
			QString(":/images/icon/ellipse.png"),
			true,
			false);
		connect(m_pEllipseAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddEllipse(bool)));

		m_pRectAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Rect"), 
			QString(":/images/icon/rectangle.png"),
			QString(":/images/icon/rectangle.png"),
			QString(":/images/icon/rectangle_press.png"), 
			QString(":/images/icon/rectangle.png"),
			true,
			false);
		connect(m_pRectAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddRect(bool)));

		m_pSectorAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Sector"), 
			QString(":/images/icon/sector.png"),
			QString(":/images/icon/sector.png"),
			QString(":/images/icon/sector_press.png"), 
			QString(":/images/icon/sector.png"),
			true,
			false);
		connect(m_pSectorAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddSector(bool)));

		m_pCircleAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Circle"), 
			QString(":/images/icon/circle.png"),
			QString(":/images/icon/circle.png"),
			QString(":/images/icon/circle_press.png"), 
			QString(":/images/icon/circle.png"),
			true,
			false);
		connect(m_pCircleAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddCircle(bool)));

		m_pTextAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Text"), 
			QString(":/images/icon/text.png"),
			QString(":/images/icon/text.png"),
			QString(":/images/icon/text_press.png"), 
			QString(":/images/icon/text.png"),
			true,
			false);
		connect(m_pTextAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddText(bool)));

		m_pLabelAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Label"), 
			QString(":/images/icon/label.png"),
			QString(":/images/icon/label.png"),
			QString(":/images/icon/label_press.png"), 
			QString(":/images/icon/label.png"),
			true,
			false);
		connect(m_pLabelAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddLabel(bool)));

		m_pModelAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Model"), 
			QString(":/images/icon/model.png"),
			QString(":/images/icon/model.png"),
			QString(":/images/icon/model_press.png"), 
			QString(":/images/icon/model.png"),
			true,
			false);
		connect(m_pModelAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddModel(bool)));

		m_pTiltModelAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("TiltModel"), 
			QString(":/images/icon/oblique_model.png"),
			QString(":/images/icon/oblique_model.png"),
			QString(":/images/icon/oblique_model_press.png"), 
			QString(":/images/icon/oblique_model.png"),
			true,
			false);
		connect(m_pTiltModelAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddTiltModel(bool)));

		m_pBillboardAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Billboard"), 
			QString(":/images/icon/billboard.png"),
			QString(":/images/icon/billboard.png"),
			QString(":/images/icon/billboard_press.png"), 
			QString(":/images/icon/billboard.png"),
			true,
			false);
		connect(m_pBillboardAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddBillboard(bool)));

		m_pOverlayAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Overlay"), 
			QString(":/images/icon/cover.png"),
			QString(":/images/icon/cover.png"),
			QString(":/images/icon/cover_press.png"), 
			QString(":/images/icon/cover.png"),
			true,
			false);
		connect(m_pOverlayAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddOverlay(bool)));

		m_pParticleAction = m_pMarkPlugin->CreateMenuAndToolAction(
			tr("Particle"), 
			QString(":/images/icon/particle.png"),
			QString(":/images/icon/particle.png"),
			QString(":/images/icon/particle_press.png"), 
			QString(":/images/icon/particle.png"),
			true,
			false);
		connect(m_pParticleAction, SIGNAL(triggered(bool)), this, SLOT(SlotAddParticle(bool)));
	}

	QMenu* CMarkMenuMgr::CreatePopMenu( FeExtNode::CExternNode* pExternNode, QWidget* pMenuParent )
	{
		if(!m_pMarkPlugin) return NULL;

		QMenu* pPopMenu = new QMenu(pMenuParent);
		
		pPopMenu->addMenu(CreateAddMenu(pMenuParent));

		if (pExternNode && pExternNode->AsComposeNode())
		{
			pPopMenu->addAction(m_pClearMarkAction);

			if(pExternNode != m_pMarkPlugin->GetMarkSys()->GetRootMark())
			{
				pPopMenu->addAction(m_pDeleteMarkAction);
			}
		}
		else
		{
			pPopMenu->addAction(m_pDeleteMarkAction);
		}

		pPopMenu->addAction(m_pOpenPropertyAction);

		return pPopMenu;
	}

	QMenu* CMarkMenuMgr::CreateRootPopMenu( FeExtNode::CExternNode* pExternNode, QWidget* pMenuParent )
	{
		if(!m_pMarkPlugin) return NULL;

		QMenu* pPopMenu = new QMenu(pMenuParent);

		if (pExternNode && pExternNode->AsComposeNode())
		{
			m_pFolderAction->setText(tr("Add Folder"));
			pPopMenu->addAction(m_pFolderAction);
			pPopMenu->addAction(m_pClearMarkAction);
		}

		return pPopMenu;
	}

	QMenu* CMarkMenuMgr::CreateAddMenu(QWidget* pMenuParent)
	{
		QMenu* pAddLayerMenu = new QMenu(tr("Add Mark"), pMenuParent);
		m_pFolderAction->setText(tr("Folder"));
		pAddLayerMenu->addAction(m_pFolderAction);
		pAddLayerMenu->addAction(m_pPointAction);
		pAddLayerMenu->addAction(m_pLineAction);
		pAddLayerMenu->addAction(m_pPolygonAction);
		//pAddLayerMenu->addAction(m_pStraightAction);
		//pAddLayerMenu->addAction(m_pAssaultAction);
		//pAddLayerMenu->addAction(m_pAttackAction);
		pAddLayerMenu->addAction(m_pModelAction);
		pAddLayerMenu->addAction(m_pBillboardAction);
		pAddLayerMenu->addAction(m_pOverlayAction);
		pAddLayerMenu->addAction(m_pArcAction);
		pAddLayerMenu->addAction(m_pEllipseAction);
		pAddLayerMenu->addAction(m_pRectAction);
		pAddLayerMenu->addAction(m_pSectorAction);
		pAddLayerMenu->addAction(m_pCircleAction);
		pAddLayerMenu->addAction(m_pTextAction);
		pAddLayerMenu->addAction(m_pTiltModelAction);
		pAddLayerMenu->addAction(m_pParticleAction);
		pAddLayerMenu->addAction(m_pLabelAction);
		return pAddLayerMenu;
	}
	
	void CMarkMenuMgr::HandleMarkAction( osg::ref_ptr<FeExtNode::CExternNode> pNode, QAction* pAction, bool bAction )
	{
		if(!m_pMarkPlugin) return;

		m_pMarkPlugin->GetMainWindow()->ClearMeasure();
		m_pMarkPlugin->GetMainWindow()->ClearPlot();

		if(!bAction)
		{
			m_pMarkPlugin->StopEventCapture();
			m_pMarkPlugin->FailedToDraw();
			SetMutex(NULL);
		}
		else
		{
			if(m_pMarkPlugin->IsDrawingOrEditing())
			{
				if(pAction) pAction->setChecked(false);
				m_pMarkPlugin->FailedToDraw();
				return;
			}
		
			if(pNode)
			{
				m_pMarkPlugin->GetMarkProcessor().DoProcess(pNode, CMarkVisitProcessor::E_MARK_INITIALIZE);
				m_pMarkPlugin->SetActiveMarkNode(pNode);
				SetMutex(pAction);
				m_pMarkPlugin->StartEventCapture();
			}
		}
	}

	void CMarkMenuMgr::SlotAddFolder( bool bAction )
	{
		if(bAction && m_pMarkPlugin)
		{
			m_pMarkPlugin->GetMainWindow()->ClearMeasure();
			m_pMarkPlugin->GetMainWindow()->ClearPlot();

			if(m_pFolderAction) m_pFolderAction->setChecked(false);

			if(m_pMarkPlugin->IsDrawingOrEditing())
			{
				m_pMarkPlugin->FailedToDraw();
				return;
			}

			FeExtNode::CExternNode* pNode = new FeExtNode::CExComposeNode(new FeExtNode::CExComposeNodeOption());
			m_pMarkPlugin->GetMarkProcessor().DoProcess(pNode, CMarkVisitProcessor::E_MARK_INITIALIZE);

			m_pMarkPlugin->SetActiveMarkNode(pNode);
			m_pMarkPlugin->StartDrawMark();
			m_pMarkPlugin->SuccessToDraw();
		}
	}

	void CMarkMenuMgr::SlotAddPoint(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExPlaceNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExPlaceNodeOption()) 
				: NULL, 
				m_pPointAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddLine(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExLineNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExLineNodeOption()) 
				: NULL, 
				m_pLineAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddPolygon(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExPolygonNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExPolygonNodeOption()) 
				: NULL, 
				m_pPolygonAction, bAction);
		}
	}

	/*
	void CMarkMenuMgr::SlotAddStraight(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExStraightArrowNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExStraightArrowNodeOption()) 
				: NULL, 
				m_pStraightAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddAssault(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExAssaultNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExAssaultNodeOption()) 
				: NULL, 
				m_pAssaultAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddAttack(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExAttackNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExAttackNodeOption()) 
				: NULL, 
				m_pAttackAction, bAction);
		}
	}
	*/

	void CMarkMenuMgr::SlotAddArc(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExArcNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExArcNodeOption()) 
				: NULL, 
				m_pArcAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddEllipse(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExEllipseNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExEllipseNodeOption()) 
				: NULL, 
				m_pEllipseAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddRect(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExRectNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExRectNodeOption()) 
				: NULL, 
				m_pRectAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddSector(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExSectorNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExSectorNodeOption()) 
				: NULL, 
				m_pSectorAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddCircle(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExCircleNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExCircleNodeOption()) 
				: NULL, 
				m_pCircleAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddText(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExTextNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExTextNodeOption()) 
				: NULL, 
				m_pTextAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddLabel(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExLabelNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExLabelNodeOption()) 
				: NULL, 
				m_pLabelAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddModel(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExLodModelNode(new FeExtNode::CExLodModelNodeOption(), m_pMarkPlugin->GetSystemService()->GetRenderContext()) 
				: NULL, 
				m_pModelAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddTiltModel(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			m_pMarkPlugin->FailedToDraw();
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExTiltModelNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExTiltModelNodeOption()) 
				: NULL, 
				m_pTiltModelAction, bAction);

			if(bAction)
			{
				m_pMarkPlugin->StartDrawMark();
				m_pMarkPlugin->SuccessToDraw();
			}
		}
	}

	void CMarkMenuMgr::SlotAddBillboard(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExBillBoardNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExBillBoardNodeOption()) 
				: NULL, 
				m_pBillboardAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddOverlay(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExOverLayNode(m_pMarkPlugin->GetSystemService()->GetRenderContext(), new FeExtNode::CExOverLayNodeOption()) 
				: NULL, 
				m_pOverlayAction, bAction);
		}
	}

	void CMarkMenuMgr::SlotAddParticle(bool bAction)
	{
		if(m_pMarkPlugin)
		{
			HandleMarkAction( (bAction && !m_pMarkPlugin->IsDrawingOrEditing()) ? 
				new FeExtNode::CExParticleNode(new FeExtNode::CExParticleNodeOption(), m_pMarkPlugin->GetSystemService()->GetRenderContext()) 
				: NULL, 
				m_pParticleAction, bAction);
		}
	}

	void CMarkMenuMgr::SetMutex( QAction* pCurrentAction )
	{
		if(m_pActiveAction)
		{
			m_pActiveAction->setChecked(false);
		}

		m_pActiveAction = pCurrentAction;

		// 状态栏操作提示
		CFreeStatusBar *pStatusBar = m_pMarkPlugin->GetMainWindow()->GetStatusBar();
		if(pStatusBar)
		{
			pStatusBar->SetMessageText(m_pActiveAction ?
				QString(tr("Tips: click the left mouse button to add point, and right button to end mark")) : "");
		}
	}
}


