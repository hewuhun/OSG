#include <QWidget>
#include <QMainWindow>

#include <FeUtils/StrUtil.h>
#include <FeExtNode/ExLineNode.h>
#include <FeExtNode/ExPolygonNode.h>
#include <FeExtNode/ExBillBoardNode.h>
#include <FeExtNode/ExOverLayNode.h>
#include <FeExtNode/ExPlaceNode.h>
#include <FeExtNode/ExTextNode.h>
#include <FeExtNode/ExSectorNode.h>
#include <FeExtNode/ExEllipseNode.h>
#include <FeExtNode/ExRectNode.h>
#include <FeExtNode/ExCircleNode.h>
#include <FeExtNode/ExTextNode.h>
#include <FeExtNode/ExLabelNode.h>
//#include <FeExtNode/ExAssaultNode.h>
//#include <FeExtNode/ExAttackNode.h>
#include <FeExtNode/ExParticleNode.h>
#include <FeExtNode/ExComposeNode.h>

#include <mark/ModelStyleWidget.h>
#include <mark/TiltModelStyleWidget.h>
#include <mark/PropertyWidget.h>
#include <mark/MarkPropertyBuilder.h>
#include <mark/ViewPointStyleWidget.h>
#include <mark/DescribeStyleWidget.h>
#include <mark/PositionStyleWidget.h>
#include <mark/LineStyleWidget.h>
#include <mark/BillboardStyleWidget.h>
#include <mark/OverLayStyleWidget.h>
#include <mark/ArcStyleWidget.h>
#include <mark/EllipseStyleWidget.h>
#include <mark/RectStyleWidget.h>
#include <mark/SectorStyleWidget.h>
#include <mark/CircleStyleWidget.h>
#include <mark/TextStyleWidget.h>
#include <mark/LabelStyleWidget.h>
#include <mark/ParticleStyleWidget.h>


namespace FreeViewer
{
	CMarkPropertyBuilder::CMarkPropertyBuilder(
		FeShell::CSystemService* pSystemService, 
		FeExtNode::CExternNode* pMark,
		bool bCreate)
		:FeExtNode::CExternNodeVisitor()
		,m_bCreate(bCreate)
		,m_opSystemService(pSystemService)
		,m_pPropertyWidget(NULL)
		,m_opExternNode(pMark)
	{
		
	}

	CMarkPropertyBuilder::~CMarkPropertyBuilder()
	{

	}

	QWidget* CMarkPropertyBuilder::CreatePropertyWidget(QWidget* pParent )
	{
		if(m_opExternNode.valid())
		{
			m_pPropertyWidget = new CPropertyWidget(m_opExternNode.get(), pParent, m_bCreate);
			
			if(BuildCommon()) 
			{
				m_opExternNode->Accept(*this);
			}
		}

		return m_pPropertyWidget;
	}

	bool CMarkPropertyBuilder::BuildCommon()
	{
		bool bComposeNode = true;			//是否为文件夹
		if(m_opExternNode.valid())
		{
			/// 文件夹不添加视点信息
			if(!dynamic_cast<FeExtNode::CExComposeNode*>(m_opExternNode.get()))
			{
				CViewPointStyleWidget* pViewPointTab = new CViewPointStyleWidget(m_opSystemService.get(),
					m_opExternNode.get(),m_bCreate,m_pPropertyWidget);
				m_pPropertyWidget->AddTab(pViewPointTab);

				//改变倾斜摄影模型位置 
				QObject::connect(m_pPropertyWidget, SIGNAL(SignalViewPointChanged(CFreeViewPoint)), pViewPointTab, SLOT(SlotChangedViwePointByMode(CFreeViewPoint)));
			}
			else
			{
				bComposeNode = false;
				m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_COMPOSE_NODE);
			}
		
			CDescribeStyleWidget* pDescribeTab = new CDescribeStyleWidget(m_opExternNode.get(),m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pDescribeTab);

			return bComposeNode;
		}

		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExComposeNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_COMPOSE_NODE);
			return true;
		}

		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExPlaceNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_PLACE_NODE);

			CPositionStyleWidget* pPosition = new CPositionStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPosition);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExLodModelNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_MODEL_NODE);

			CModelStyleWidget *pModelTab = new CModelStyleWidget(m_opSystemService.get(),
				&externNode,m_bCreate,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pModelTab);
			//m_CurrentModelTab = pModelTab;

			//设置model界面大小
			m_pPropertyWidget->resize(400, 680);

			return true;
		}
		return false;
	}


	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExLineNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_LINE_NODE);

			CLineStyleWidget *pLineTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pLineTab);
			pLineTab->setPolygonStyleVisible(false);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExPolygonNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_POLYGON_NODE);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(true);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	/*
	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExStraightArrowNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_STRING_ARROW_NODE);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(false);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExAssaultNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_ASSAULT_NODE);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(false);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExAttackNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_ATTACK_NODE);

			CLineStyleWidget *pPolygonTab = new CLineStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pPolygonTab);
			pPolygonTab->setPolygonStyleVisible(false);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}
	*/

	bool CMarkPropertyBuilder::VisitEnter(FeExtNode::CExBillBoardNode& externNode)
	{
		if (m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_BILL_BOARD_NODE);

			CBillboardStyleWidget *pMiliMarkTab = new CBillboardStyleWidget(m_opSystemService.get(), 
				&externNode, m_bCreate,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pMiliMarkTab);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter(FeExtNode::CExOverLayNode& externNode)
	{
		if (m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_OVERLAY_NODE);

			COverLayStyleWidget *pMiliMarkTab = new COverLayStyleWidget(m_opSystemService.get(), 
				&externNode, m_bCreate,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pMiliMarkTab);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}
	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExArcNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_ARC_NODE);

			CArcStyleWidget* pArcTab = new CArcStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pArcTab);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExEllipseNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_ELLIPSE_NODE);

			CEllipseStyleWidget* pEllipseTab = new CEllipseStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pEllipseTab);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExRectNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_RECT_NODE);

			CRectStyleWidget* pRectTab = new CRectStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pRectTab);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExSectorNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_SECTOR_NODE);

			CSectorStyleWidget* pSectorTab = new CSectorStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pSectorTab);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExCircleNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_CIRCLE_NODE);

			CCircleStyleWidget* pCircleTab = new CCircleStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pCircleTab);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExTextNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_TEXT_NODE);

			CTextStyleWidget* pText = new CTextStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pText);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExLabelNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_LABEL_NODE);

			CLabelStyleWidget* plabel = new CLabelStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(plabel);

			//设置label界面大小
			m_pPropertyWidget->resize(540, 570);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExTiltModelNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_TILT_MODEL_NODE);

			CTiltModelStyleWidget *pTiltModelTab = new CTiltModelStyleWidget(m_opSystemService.get(),
				&externNode, m_bCreate, m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pTiltModelTab);

			//m_CurrentTiltModelTab = pTiltModelTab;
			if (m_bCreate)
			{
				QObject::connect(pTiltModelTab, SIGNAL(SignalViewPointChanged(CFreeViewPoint)) , m_pPropertyWidget, SIGNAL(SignalViewPointChanged(CFreeViewPoint)));
			}
			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

	bool CMarkPropertyBuilder::VisitEnter( FeExtNode::CExParticleNode& externNode )
	{
		if(m_pPropertyWidget)
		{
			m_pPropertyWidget->SetTitleFlag(CPropertyWidget::E_PARTICLE_NODE);

			CParticleStyleWidget *pParticleTab = new CParticleStyleWidget(&externNode,m_pPropertyWidget);
			m_pPropertyWidget->AddTab(pParticleTab);

			m_pPropertyWidget->resize(400, 480);

			return true;
		}
		return false;
	}

}
